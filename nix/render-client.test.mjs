import assert from 'node:assert/strict';
import { mkdtempSync, mkdirSync, readFileSync, realpathSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import test from 'node:test';

const script = fileURLToPath(new URL('./render-client.mjs', import.meta.url));
const template = readFileSync(new URL('../Web/Client/index.html', import.meta.url), 'utf8');

function render(t, clientUrl, html = template) {
    const root = mkdtempSync(join(tmpdir(), 'hoverrace-metadata-test-'));
    t.after(() => rmSync(root, { recursive: true, force: true }));
    const source = join(root, 'source');
    const destination = join(root, 'rendered');
    mkdirSync(source);
    mkdirSync(destination);
    writeFileSync(join(source, 'index.html'), html);
    writeFileSync(join(source, 'hoverrace.wasm'), 'fixture');
    const result = spawnSync(process.execPath, [script, source, destination], {
        env: { ...process.env, CLIENT_URL: clientUrl }, encoding: 'utf8',
    });
    return { result, source, destination };
}

for (const [input, expected] of [
    ['https://hover.paulspiteri.com', 'https://hover.paulspiteri.com'],
    ['http://localhost:8080/', 'http://localhost:8080'],
    [' https://game.example.com/ ,https://other.example.com', 'https://game.example.com'],
]) {
    test(`renders CLIENT_URL=${input}`, t => {
        const { result, source, destination } = render(t, input);
        assert.equal(result.status, 0, result.stderr);
        const html = readFileSync(join(destination, 'index.html'), 'utf8');
        assert.ok(html.includes(`property="og:url" content="${expected}"`));
        assert.ok(html.includes(`property="og:image" content="${expected}/ogimage.png"`));
        assert.equal(readFileSync(join(source, 'index.html'), 'utf8'), template);
        assert.equal(realpathSync(join(destination, 'hoverrace.wasm')), join(source, 'hoverrace.wasm'));
    });
}

for (const input of ['', 'not-a-url', 'javascript:alert(1)']) {
    test(`rejects invalid CLIENT_URL=${input}`, t => {
        assert.notEqual(render(t, input).result.status, 0);
    });
}

test('fails if the metadata template changes unexpectedly', t => {
    assert.notEqual(render(t, 'https://game.example.com', '<html></html>').result.status, 0);
});
