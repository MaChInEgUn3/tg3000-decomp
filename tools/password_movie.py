#!/usr/bin/env python3
"""Type a password on the game's password screen, as bench movie lines.

    python3 -I tools/password_movie.py PREFIX TEXT START > movie.txt

PREFIX: a movie that reaches the password screen with the cursor on A (race1.txt up to its
Championship menu, then Down x4 and A: docs/DECISIONS.md "A password movie"). TEXT: the
password, from the game's alphabet. START: the first step to type at.

The grid: A-L and SPACE, M-X and DEL, Y Z 0-9 and EXIT, then . ? ! (a row of three). A tap
is held 6 steps, one every 16. The last row is reached from its own columns and left
upwards first. EXIT ends it: the cursor glides there, so its A comes 60 steps later.
"""
import sys

ROWS = ["ABCDEFGHIJKL", "MNOPQRSTUVWX", "YZ0123456789", ".?!"]
POS = {c: (r, k) for r, line in enumerate(ROWS) for k, c in enumerate(line)}
POS["EXIT"] = (2, 12)
STEP, HOLD, EXIT_WAIT = 16, 6, 60


def taps(text, frame):
    out = []
    row = col = 0
    for ch in list(text) + ["EXIT"]:
        to_row, to_col = POS[ch]

        def vertical():
            nonlocal row, frame
            while row != to_row:
                out.append((frame, "D" if to_row > row else "U"))
                row += 1 if to_row > row else -1
                frame += STEP

        def horizontal():
            nonlocal col, frame
            while col != to_col:
                out.append((frame, "R" if to_col > col else "L"))
                col += 1 if to_col > col else -1
                frame += STEP

        if to_row == 3:
            horizontal()
            vertical()
        else:
            vertical()
            horizontal()
        if ch == "EXIT":
            frame += EXIT_WAIT
        out.append((frame, "A"))
        frame += STEP
    return out


def main():
    prefix, text, start = sys.argv[1], sys.argv[2], int(sys.argv[3])
    for ch in text:
        if ch not in POS:
            sys.exit(f"not on the grid: {ch!r}")
    with open(prefix) as f:
        sys.stdout.write("".join(l for l in f if not l.startswith("#")))
    for frame, button in taps(text, start):
        print(frame, button)
        print(frame + HOLD, ".")


if __name__ == "__main__":
    main()
