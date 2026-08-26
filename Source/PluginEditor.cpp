#include "PluginEditor.h"
#include "BinaryData.h"

namespace photosynth
{
    namespace
    {
        juce::String mimeTypeForPath(const juce::String& path)
        {
            if (path.endsWithIgnoreCase(".html")) return "text/html; charset=utf-8";
            if (path.endsWithIgnoreCase(".css")) return "text/css; charset=utf-8";
            if (path.endsWithIgnoreCase(".js")) return "application/javascript; charset=utf-8";
            if (path.endsWithIgnoreCase(".json")) return "application/json; charset=utf-8";
            if (path.endsWithIgnoreCase(".png")) return "image/png";
            if (path.endsWithIgnoreCase(".jpg") || path.endsWithIgnoreCase(".jpeg")) return "image/jpeg";
            if (path.endsWithIgnoreCase(".svg")) return "image/svg+xml";
            if (path.endsWithIgnoreCase(".webp")) return "image/webp";
            return "application/octet-stream";
        }

        juce::String normaliseWebPath(juce::String p)
        {
            if (p.isEmpty() || p == "/")
                return "index.html";

            if (p.startsWithChar('/'))
                p = p.substring(1);

            const auto query = p.indexOfChar('?');
            if (query >= 0)
                p = p.substring(0, query);

            return p;
        }

        juce::var parseJsonSafely(const juce::String& text)
        {
            juce::var parsed;
            auto result = juce::JSON::parse(text, parsed);
            if (result.failed())
                return {};
            return parsed;
        }

        void writeHttpResponse(juce::StreamingSocket& socket,
                               int statusCode,
                               const juce::String& statusText,
                               const juce::String& contentType,
                               const juce::MemoryBlock& body)
        {
            juce::String headers;
            headers << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n"
                    << "Connection: close\r\n"
                    << "Content-Type: " << contentType << "\r\n"
                    << "Content-Length: " << (int) body.getSize() << "\r\n"
                    << "Access-Control-Allow-Origin: *\r\n"
                    << "\r\n";

            socket.write(headers.toRawUTF8(), (int) headers.getNumBytesAsUTF8());
            if (body.getSize() > 0)
                socket.write(body.getData(), (int) body.getSize());
        }

        juce::MemoryBlock toBody(const juce::String& text)
        {
            juce::MemoryBlock out;
            out.append(text.toRawUTF8(), (size_t) text.getNumBytesAsUTF8());
            return out;
        }

        juce::String bridgeScript()
        {
            return R"JS(
(function () {
  const bridge = {
    async setParam(id, value) {
      try {
        await fetch('/bridge/param', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({ id, value })
        });
      } catch (_) {}
    },
    async noteOn(note, velocity = 0.8) {
      try {
        await fetch('/bridge/note', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({ on: true, note, velocity })
        });
      } catch (_) {}
    },
    async noteOff(note) {
      try {
        await fetch('/bridge/note', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({ on: false, note })
        });
      } catch (_) {}
    },
    async panic() {
      try { await fetch('/bridge/panic', { method: 'POST' }); } catch (_) {}
    },
    async pushState(snapshot) {
      if (!snapshot || typeof snapshot !== 'object') return;
      try {
        if (snapshot.patch) {
          for (const [id, value] of Object.entries(snapshot.patch)) {
            if (typeof value === 'number')
              await bridge.setParam(id, value);
          }
        }
        if (typeof snapshot.morphValue === 'number')
          await bridge.setParam('morphAmount', snapshot.morphValue);
        if (typeof snapshot.bpm === 'number')
          await bridge.setParam('bpm', snapshot.bpm);
      } catch (_) {}
    }
  };

  window.__PHOTO_SYNTH_HOST__ = bridge;

  setInterval(async () => {
    try {
      const r = await fetch('/bridge/state');
      if (!r.ok) return;
      const state = await r.json();
      window.dispatchEvent(new CustomEvent('juce-bridge-state', { detail: state }));
    } catch (_) {}
  }, 250);
})();
)JS";
        }
    }

    class PhotoSynthAudioProcessorEditor::WebUiServer : private juce::Thread
    {
    public:
        explicit WebUiServer(PhotoSynthAudioProcessor& p)
            : juce::Thread("PhotoSynthWebUiServer"), processor(p)
        {
            listener = std::make_unique<juce::StreamingSocket>();
            for (int port = 49750; port < 49950; ++port)
            {
                if (listener->createListener(port, "127.0.0.1"))
                {
                    boundPort = port;
                    break;
                }
            }

            if (boundPort > 0)
                startThread();
        }

        ~WebUiServer() override
        {
            signalThreadShouldExit();
            if (listener != nullptr)
                listener->close();
            stopThread(1500);
        }

        juce::String getUrl() const
        {
            if (boundPort <= 0)
                return {};
            return "http://127.0.0.1:" + juce::String(boundPort) + "/index.html";
        }

        bool isRunningOk() const { return boundPort > 0; }

    private:
        PhotoSynthAudioProcessor& processor;
        std::unique_ptr<juce::StreamingSocket> listener;
        int boundPort = -1;

        const char* findAssetData(const juce::String& relativePath, int& dataSize)
        {
            for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
            {
                if (juce::String(BinaryData::originalFilenames[i]) == relativePath)
                    return BinaryData::getNamedResource(BinaryData::namedResourceList[i], dataSize);
            }
            return nullptr;
        }

        void run() override
        {
            while (!threadShouldExit())
            {
                if (listener == nullptr)
                    break;

                std::unique_ptr<juce::StreamingSocket> client(listener->waitForNextConnection());
                if (client == nullptr)
                    continue;

                handleClient(*client);
            }
        }

        void handleClient(juce::StreamingSocket& socket)
        {
            juce::MemoryBlock requestData;
            char buffer[8192]{};

            while (true)
            {
                auto bytes = socket.read(buffer, (int) sizeof(buffer), false);
                if (bytes <= 0)
                    break;
                requestData.append(buffer, (size_t) bytes);
                if (bytes < (int) sizeof(buffer))
                    break;
            }

            const auto request = juce::String::fromUTF8(static_cast<const char*>(requestData.getData()), (int) requestData.getSize());
            if (request.isEmpty())
                return;

            auto lines = juce::StringArray::fromLines(request);
            if (lines.isEmpty())
                return;

            auto requestLine = juce::StringArray::fromTokens(lines[0], " ", "");
            if (requestLine.size() < 2)
                return;

            const auto method = requestLine[0].trim();
            const auto fullPath = requestLine[1].trim();
            const auto headerEnd = request.indexOf("\r\n\r\n");
            juce::String body;
            if (headerEnd >= 0)
                body = request.substring(headerEnd + 4);

            if (method == "POST")
            {
                if (fullPath.startsWith("/bridge/param"))
                {
                    auto payload = parseJsonSafely(body);
                    if (auto* obj = payload.getDynamicObject())
                    {
                        const auto id = obj->getProperty("id").toString();
                        const auto value = (double) obj->getProperty("value");
                        processor.setParameterFromBridge(id, value);
                    }
                    writeHttpResponse(socket, 200, "OK", "application/json", toBody("{\"ok\":true}"));
                    return;
                }

                if (fullPath.startsWith("/bridge/note"))
                {
                    auto payload = parseJsonSafely(body);
                    if (auto* obj = payload.getDynamicObject())
                    {
                        const bool noteOn = (bool) obj->getProperty("on");
                        const int note = (int) obj->getProperty("note");
                        const float velocity = (float) (double) obj->getProperty("velocity");
                        if (noteOn)
                            processor.playNoteFromBridge(note, velocity > 0.0f ? velocity : 0.8f);
                        else
                            processor.releaseNoteFromBridge(note);
                    }
                    writeHttpResponse(socket, 200, "OK", "application/json", toBody("{\"ok\":true}"));
                    return;
                }

                if (fullPath.startsWith("/bridge/panic"))
                {
                    processor.panicFromBridge();
                    writeHttpResponse(socket, 200, "OK", "application/json", toBody("{\"ok\":true}"));
                    return;
                }
            }

            if (method == "GET")
            {
                if (fullPath.startsWith("/bridge/state"))
                {
                    const auto json = juce::JSON::toString(processor.createBridgeState());
                    writeHttpResponse(socket, 200, "OK", "application/json", toBody(json));
                    return;
                }

                if (fullPath.startsWith("/juce-bridge.js"))
                {
                    writeHttpResponse(socket, 200, "OK", "application/javascript", toBody(bridgeScript()));
                    return;
                }

                const auto rel = normaliseWebPath(fullPath);
                int dataSize = 0;
                if (const auto* data = findAssetData(rel, dataSize))
                {
                    juce::MemoryBlock out;
                    out.append(data, (size_t) dataSize);

                    if (rel == "index.html")
                    {
                        auto html = juce::String::fromUTF8(static_cast<const char*>(out.getData()), (int) out.getSize());
                        if (!html.contains("/juce-bridge.js"))
                            html = html.replace("</head>", "  <script src=\"/juce-bridge.js\"></script>\n</head>");
                        writeHttpResponse(socket, 200, "OK", mimeTypeForPath(rel), toBody(html));
                    }
                    else
                    {
                        writeHttpResponse(socket, 200, "OK", mimeTypeForPath(rel), out);
                    }
                    return;
                }
            }

            writeHttpResponse(socket, 404, "Not Found", "text/plain", toBody("Not found"));
        }
    };

    PhotoSynthAudioProcessorEditor::PhotoSynthAudioProcessorEditor(PhotoSynthAudioProcessor& p)
        : AudioProcessorEditor(&p), processor(p)
    {
        setSize(1280, 849);

        statusLabel.setJustificationType(juce::Justification::centred);
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        addAndMakeVisible(statusLabel);

        server = std::make_unique<WebUiServer>(processor);

        if (server->isRunningOk() && juce::WebBrowserComponent::areOptionsSupported({}))
        {
            browser = std::make_unique<juce::WebBrowserComponent>(juce::WebBrowserComponent::Options().withKeepPageLoadedWhenBrowserIsHidden());
            addAndMakeVisible(*browser);
            browser->goToURL(server->getUrl());
            statusLabel.setText("Web UI: " + server->getUrl(), juce::dontSendNotification);
        }
        else
        {
            statusLabel.setText("Failed to start embedded WebView server", juce::dontSendNotification);
        }

        startTimerHz(20);
    }

    PhotoSynthAudioProcessorEditor::~PhotoSynthAudioProcessorEditor()
    {
        stopTimer();
        browser.reset();
        server.reset();
    }

    void PhotoSynthAudioProcessorEditor::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colour(0xff101318));
    }

    void PhotoSynthAudioProcessorEditor::resized()
    {
        auto area = getLocalBounds();
        auto top = area.removeFromTop(22);
        statusLabel.setBounds(top.reduced(6, 0));

        if (browser != nullptr)
            browser->setBounds(area.reduced(2));
    }

    void PhotoSynthAudioProcessorEditor::timerCallback()
    {
        if (statusLabel.getText().startsWith("Web UI:") && browser == nullptr)
            statusLabel.setText("Web UI initialized", juce::dontSendNotification);
    }
}
