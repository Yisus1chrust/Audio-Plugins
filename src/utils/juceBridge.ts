import { PatchParameters } from '../types/synth';

export interface JuceBridgeState {
  patch?: Partial<PatchParameters>;
  metrics?: Record<string, unknown>;
  morphAmount?: number;
  bpm?: number;
  hasImageA?: boolean;
  hasImageB?: boolean;
}

const isEmbedded = () => typeof window !== 'undefined' && !!(window as any).__PHOTO_SYNTH_HOST__;

async function postJson(path: string, payload: unknown): Promise<void> {
  if (!isEmbedded()) return;
  try {
    await fetch(path, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload),
    });
  } catch {
    // Ignore bridge transport failures and keep web app functional.
  }
}

export async function pushPatchToHost(patch: Partial<PatchParameters> | null): Promise<void> {
  if (!patch) return;
  for (const [id, value] of Object.entries(patch)) {
    if (typeof value === 'number') {
      await postJson('/bridge/param', { id, value });
    }
  }
}

export function pushTransportToHost(morphValue: number, bpm: number): void {
  void postJson('/bridge/param', { id: 'morphAmount', value: morphValue });
  void postJson('/bridge/param', { id: 'bpm', value: bpm });
}

export function noteOnToHost(note: number, velocity = 0.8): void {
  void postJson('/bridge/note', { on: true, note, velocity });
}

export function noteOffToHost(note: number): void {
  void postJson('/bridge/note', { on: false, note });
}

export function panicToHost(): void {
  if (!isEmbedded()) return;
  fetch('/bridge/panic', { method: 'POST' }).catch(() => {});
}

export function subscribeBridgeState(callback: (state: JuceBridgeState) => void): () => void {
  const handler = (event: Event) => {
    const custom = event as CustomEvent<JuceBridgeState>;
    if (custom.detail) callback(custom.detail);
  };

  window.addEventListener('juce-bridge-state', handler as EventListener);
  return () => window.removeEventListener('juce-bridge-state', handler as EventListener);
}
