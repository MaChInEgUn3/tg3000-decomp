#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Generate the asar disassembly in asm/ from the code/data map.

    python3 -I tools/listing.py

Inputs: map/code_starts.txt, map/rom_map.txt and the ROM (TG3000_ROM, default
./TG3000.sfc). Output: asm/main.asm plus asm/bank_XX.asm, one per 32 KB LoROM bank.

- Executed code becomes 65816 instructions with explicit operand sizes (.b/.w/.l), so
  asar reproduces the exact bytes; branches use labels (CODE_bbaaaa).
- Code bytes that cannot be emitted as an instruction (register widths ambiguous for a
  width-dependent opcode, or crossing a bank) become `db`.
- Everything else (data, and bytes never seen executed or read) is not copied into the
  repository: it is `incbin`'d from your own ROM at build time.

tools/build_asm.sh assembles it and checks the SHA-1 against the original.
"""
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROM_SIZE, BANK = 0x100000, 0x8000

# (mnemonic, mode) for every opcode. Modes: imp acc immm immx imm8 dp dpx dpy idp idpx idpy
# ildp ildpy abs absx absy long longx iabs iabsx ilabs sr isry rel rell blk pea pei per
OPS = """
brk imm8|ora idpx|cop imm8|ora sr|tsb dp|ora dp|asl dp|ora ildp|php imp|ora immm|asl acc|phd imp|tsb abs|ora abs|asl abs|ora long
bpl rel|ora idpy|ora idp|ora isry|trb dp|ora dpx|asl dpx|ora ildpy|clc imp|ora absy|inc acc|tcs imp|trb abs|ora absx|asl absx|ora longx
jsr abs|and idpx|jsl long|and sr|bit dp|and dp|rol dp|and ildp|plp imp|and immm|rol acc|pld imp|bit abs|and abs|rol abs|and long
bmi rel|and idpy|and idp|and isry|bit dpx|and dpx|rol dpx|and ildpy|sec imp|and absy|dec acc|tsc imp|bit absx|and absx|rol absx|and longx
rti imp|eor idpx|wdm imm8|eor sr|mvp blk|eor dp|lsr dp|eor ildp|pha imp|eor immm|lsr acc|phk imp|jmp abs|eor abs|lsr abs|eor long
bvc rel|eor idpy|eor idp|eor isry|mvn blk|eor dpx|lsr dpx|eor ildpy|cli imp|eor absy|phy imp|tcd imp|jml long|eor absx|lsr absx|eor longx
rts imp|adc idpx|per per|adc sr|stz dp|adc dp|ror dp|adc ildp|pla imp|adc immm|ror acc|rtl imp|jmp iabs|adc abs|ror abs|adc long
bvs rel|adc idpy|adc idp|adc isry|stz dpx|adc dpx|ror dpx|adc ildpy|sei imp|adc absy|ply imp|tdc imp|jmp iabsx|adc absx|ror absx|adc longx
bra rel|sta idpx|brl rell|sta sr|sty dp|sta dp|stx dp|sta ildp|dey imp|bit immm|txa imp|phb imp|sty abs|sta abs|stx abs|sta long
bcc rel|sta idpy|sta idp|sta isry|sty dpx|sta dpx|stx dpy|sta ildpy|tya imp|sta absy|txs imp|txy imp|stz abs|sta absx|stz absx|sta longx
ldy immx|lda idpx|ldx immx|lda sr|ldy dp|lda dp|ldx dp|lda ildp|tay imp|lda immm|tax imp|plb imp|ldy abs|lda abs|ldx abs|lda long
bcs rel|lda idpy|lda idp|lda isry|ldy dpx|lda dpx|ldx dpy|lda ildpy|clv imp|lda absy|tsx imp|tyx imp|ldy absx|lda absx|ldx absy|lda longx
cpy immx|cmp idpx|rep imm8|cmp sr|cpy dp|cmp dp|dec dp|cmp ildp|iny imp|cmp immm|dex imp|wai imp|cpy abs|cmp abs|dec abs|cmp long
bne rel|cmp idpy|cmp idp|cmp isry|pei pei|cmp dpx|dec dpx|cmp ildpy|cld imp|cmp absy|phx imp|stp imp|jml ilabs|cmp absx|dec absx|cmp longx
cpx immx|sbc idpx|sep imm8|sbc sr|cpx dp|sbc dp|inc dp|sbc ildp|inx imp|sbc immm|nop imp|xba imp|cpx abs|sbc abs|inc abs|sbc long
beq rel|sbc idpy|sbc idp|sbc isry|pea pea|sbc dpx|inc dpx|sbc ildpy|sed imp|sbc absy|plx imp|xce imp|jsr iabsx|sbc absx|inc absx|sbc longx
"""
TABLE = [tuple(e.split()) for row in OPS.strip().splitlines() for e in row.split("|")]
assert len(TABLE) == 256

FIXED = {"imp": 1, "acc": 1, "imm8": 2, "dp": 2, "dpx": 2, "dpy": 2, "idp": 2, "idpx": 2, "idpy": 2,
         "ildp": 2, "ildpy": 2, "sr": 2, "isry": 2, "rel": 2, "pei": 2, "abs": 3, "absx": 3, "absy": 3,
         "iabs": 3, "iabsx": 3, "ilabs": 3, "rell": 3, "blk": 3, "pea": 3, "per": 3, "long": 4, "longx": 4}
BRANCH = {"rel", "rell"}


def width_dependent(mode):
    return mode in ("immm", "immx")


def length(op, m8, x8):
    mode = TABLE[op][1]
    if mode == "immm":
        return 2 if m8 else 3
    if mode == "immx":
        return 2 if x8 else 3
    return FIXED[mode]


def snes(off):
    """CPU address of a file offset, in the FastROM mirror ($80-$FF) the game runs from."""
    return ((0x80 | (off >> 15)) << 16) | 0x8000 | (off & 0x7FFF)


def label(addr):
    return f"CODE_{addr:06X}"


def main():
    rom_path = os.environ.get("TG3000_ROM", os.path.join(ROOT, "TG3000.sfc"))
    rom = open(rom_path, "rb").read()
    if len(rom) != ROM_SIZE:
        print("expected a 1 MB ROM", file=sys.stderr)
        return 1

    code = bytearray(ROM_SIZE)  # 1 = code byte
    for line in open(os.path.join(ROOT, "map", "rom_map.txt")):
        if line.startswith("#"):
            continue
        a, b, kind = line.split()[:3]
        if kind == "code":
            for o in range(int(a, 16), int(b, 16)):
                code[o] = 1

    starts = {}  # offset -> set of widths
    for line in open(os.path.join(ROOT, "map", "code_starts.txt")):
        if line.startswith("#"):
            continue
        off, _, widths = line.split()
        starts[int(off, 16)] = set(widths.split(","))

    def decide(off):
        """(m8, x8) to decode the instruction at off, or None if it cannot be emitted."""
        ws = starts[off] - {"emu"}
        op = rom[off]
        mode = TABLE[op][1]
        if not ws:
            return None
        if width_dependent(mode):
            key = "m" if mode == "immm" else "x"
            sizes = {w.split("x")[0][1:] if key == "m" else w.split("x")[1] for w in ws}
            if len(sizes) != 1:
                return None
        w = sorted(ws)[0]
        m8, x8 = w.startswith("m8"), w.endswith("x8")
        n = length(op, m8, x8)
        if (off + n - 1) >> 15 != off >> 15:
            return None  # crosses a bank
        return m8, x8

    # pass 1: which instructions are emitted (labels may only point at those)
    emitted = {}
    off = 0
    while off < ROM_SIZE:
        if off in starts and code[off]:
            d = decide(off)
            if d is not None:
                n = length(rom[off], *d)
                emitted[off] = (d, n)
                off += n
                continue
        off += 1
    targets = set()
    for off, (d, n) in emitted.items():
        op = rom[off]
        mode = TABLE[op][1]
        if mode in BRANCH:
            disp = rom[off + 1] if mode == "rel" else rom[off + 1] | rom[off + 2] << 8
            if mode == "rel" and disp >= 0x80:
                disp -= 0x100
            if mode == "rell" and disp >= 0x8000:
                disp -= 0x10000
            t = (off & ~0x7FFF) | ((((off & 0x7FFF) | 0x8000) + n + disp) & 0x7FFF) \
                if 0x8000 <= ((off & 0x7FFF) | 0x8000) + n + disp <= 0xFFFF else None
            if t is not None and t in emitted:
                targets.add(t)

    def operand(off, mode, n, d):
        b = rom[off + 1: off + n]
        v = int.from_bytes(b, "little")
        h2, h4, h6 = f"${v:02X}", f"${v:04X}", f"${v:06X}"
        return {
            "imp": "", "acc": "a", "imm8": f"#{h2}", "dp": h2, "dpx": f"{h2},x", "dpy": f"{h2},y",
            "idp": f"({h2})", "idpx": f"({h2},x)", "idpy": f"({h2}),y", "ildp": f"[{h2}]",
            "ildpy": f"[{h2}],y", "sr": f"{h2},s", "isry": f"({h2},s),y", "abs": h4, "absx": f"{h4},x",
            "absy": f"{h4},y", "iabs": f"({h4})", "iabsx": f"({h4},x)", "ilabs": f"[{h4}]",
            "long": h6, "longx": f"{h6},x", "pea": h4, "pei": f"({h2})",
            "blk": f"${b[0]:02X},${b[1]:02X}" if len(b) == 2 else "",
            "immm": f"#{h2 if n == 2 else h4}", "immx": f"#{h2 if n == 2 else h4}",
        }[mode]

    SUFFIX = {1: "", 2: ".b", 3: ".w", 4: ".l"}
    NO_SUFFIX = {"imp", "acc", "blk", "pei", "rel", "rell", "per"}

    out_dir = os.path.join(ROOT, "asm")
    os.makedirs(out_dir, exist_ok=True)
    stats = {"insn": 0, "db": 0, "incbin": 0}
    with open(os.path.join(out_dir, "main.asm"), "w") as main_f:
        main_f.write("; Top Gear 3000 (USA) - generated by tools/listing.py; build with tools/build_asm.sh\n"
                     "; Data and never-executed bytes are incbin'd from your ROM (!baserom).\n"
                     "lorom\n\n")
        for bank in range(ROM_SIZE // BANK):
            name = f"bank_{bank:02X}.asm"
            main_f.write(f'incsrc "{name}"\n')
            lines = [f"; bank ${0x80 | bank:02X} (file offset ${bank * BANK:06X})",
                     f"org ${snes(bank * BANK):06X}", ""]
            off, end = bank * BANK, (bank + 1) * BANK
            pending_db, pending_bin = [], None

            def flush():
                nonlocal pending_db, pending_bin
                if pending_db:
                    for i in range(0, len(pending_db), 16):
                        lines.append("    db " + ",".join(f"${x:02X}" for x in pending_db[i:i + 16]))
                    pending_db = []
                if pending_bin is not None:
                    a, b = pending_bin
                    lines.append(f"    incbin !baserom:${a:X}..${b:X}")
                    pending_bin = None

            while off < end:
                if off in emitted:
                    flush()
                    (m8, x8), n = emitted[off]
                    op = rom[off]
                    mn, mode = TABLE[op]
                    if off in targets:
                        lines.append(f"{label(snes(off))}:")
                    if mode in BRANCH:
                        disp = rom[off + 1] if mode == "rel" else rom[off + 1] | rom[off + 2] << 8
                        if mode == "rel" and disp >= 0x80:
                            disp -= 0x100
                        if mode == "rell" and disp >= 0x8000:
                            disp -= 0x10000
                        tpc = ((off & 0x7FFF) | 0x8000) + n + disp
                        t = (off & ~0x7FFF) | (tpc & 0x7FFF) if 0x8000 <= tpc <= 0xFFFF else None
                        if t is not None and t in targets:
                            text = f"{mn} {label(snes(t))}"
                        else:  # target not an emitted instruction: keep the raw bytes
                            text = "db " + ",".join(f"${x:02X}" for x in rom[off:off + n]) + f" ; {mn}"
                            stats["db"] += n
                    elif mode == "per":
                        text = "db " + ",".join(f"${x:02X}" for x in rom[off:off + n]) + " ; per"
                        stats["db"] += n
                    else:
                        suffix = "" if mode in NO_SUFFIX else SUFFIX[n]
                        opnd = operand(off, mode, n, (m8, x8))
                        text = f"{mn}{suffix} {opnd}".rstrip()
                    lines.append(f"    {text:<28}; ${snes(off):06X}")
                    stats["insn"] += 1
                    off += n
                elif code[off]:
                    if pending_bin is not None:
                        flush()
                    pending_db.append(rom[off])
                    stats["db"] += 1
                    off += 1
                else:
                    if pending_db:
                        flush()
                    pending_bin = (pending_bin[0], off + 1) if pending_bin else (off, off + 1)
                    stats["incbin"] += 1
                    off += 1
            flush()
            with open(os.path.join(out_dir, name), "w") as f:
                f.write("\n".join(lines) + "\n")
    print(f"{stats['insn']:,} instructions, {stats['db']:,} code bytes as db, "
          f"{stats['incbin']:,} bytes incbin'd from the ROM, {len(targets):,} labels")
    return 0


if __name__ == "__main__":
    sys.exit(main())
