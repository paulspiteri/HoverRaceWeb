import { readFileSync, readdirSync, symlinkSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';

// CLIENT_URL may contain several CORS origins; the first is the public site.
const publicUrl = new URL((process.env.CLIENT_URL ?? '').split(',')[0].trim());
if (!['http:', 'https:'].includes(publicUrl.protocol)) {
    throw new Error('CLIENT_URL must start with an HTTP(S) origin');
}
const origin = publicUrl.origin;
const [source, destination] = process.argv.slice(2);
let html = readFileSync(join(source, 'index.html'), 'utf8');
for (const [property, url] of [['og:url', origin], ['og:image', `${origin}/ogimage.png`]]) {
    const tag = new RegExp(`(<meta property="${property}" content=")[^"]*("\\s*/?>)`);
    if (!tag.test(html)) throw new Error(`Missing ${property} metadata`);
    html = html.replace(tag, (_, before, after) => `${before}${url}${after}`);
}
writeFileSync(join(destination, 'index.html'), html);
// Keep large WASM/data files in the immutable store instead of copying them.
for (const name of readdirSync(source)) {
    if (name !== 'index.html') symlinkSync(join(source, name), join(destination, name));
}
