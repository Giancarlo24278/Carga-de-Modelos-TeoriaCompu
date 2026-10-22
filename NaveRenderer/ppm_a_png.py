"""Convierte el PPM generado por el renderer a PNG. Solo usa Python estandar."""
import struct
import sys
import zlib
from pathlib import Path

def convert(source, destination):
    with open(source, 'rb') as f:
        if f.readline().strip() != b'P6':
            raise ValueError('Se esperaba un PPM P6 generado por este renderer')
        width, height = map(int, f.readline().split())
        if f.readline().strip() != b'255':
            raise ValueError('Se esperaba un PPM RGB de 8 bits')
        pixels = f.read()
    if len(pixels) != width * height * 3:
        raise ValueError('Tamano de imagen incorrecto')
    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data) & 0xffffffff)
    stride = width * 3
    rows = b''.join(b'\0' + pixels[y*stride:(y+1)*stride] for y in range(height))
    png = b'\x89PNG\r\n\x1a\n'
    png += chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(rows)) + chunk(b'IEND', b'')
    Path(destination).write_bytes(png)
    print(f'Guardado: {destination}')

if __name__ == '__main__':
    convert(sys.argv[1] if len(sys.argv)>1 else 'render.ppm',
            sys.argv[2] if len(sys.argv)>2 else 'render.png')
