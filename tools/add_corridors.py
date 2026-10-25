"""Write corridor versions of the tiled benchmark maps.

The big maps tile jungle.map (20 rows x 25 columns per copy). Every copy has
a deep-water border, so with deep water blocked the copies are islands.
Setting local column 10 and local row 7 of every copy to tile 8 (plain
ground) bridges each copy to its neighbours, while deep water stays blocked.

Usage: python add_corridors.py <in.map> <out.map>
Keeps the file's format: zero-padded two-digit ids, LF, no final newline.
"""
import sys

COPY_ROWS, COPY_COLS = 20, 25
CORRIDOR_COL, CORRIDOR_ROW = 10, 7
CORRIDOR_TILE = "08"


def main(src: str, dst: str) -> None:
    with open(src, "r", newline="") as f:
        lines = f.read().split("\n")

    changed = 0
    out_lines = []
    for row, line in enumerate(lines):
        if not line.strip():
            out_lines.append(line)
            continue
        tokens = line.split(",")
        row_is_corridor = row % COPY_ROWS == CORRIDOR_ROW
        for col in range(len(tokens)):
            if row_is_corridor or col % COPY_COLS == CORRIDOR_COL:
                if tokens[col] != CORRIDOR_TILE:
                    changed += 1
                tokens[col] = CORRIDOR_TILE
        out_lines.append(",".join(tokens))

    with open(dst, "w", newline="") as f:
        f.write("\n".join(out_lines))
    print(f"{dst}: {changed} tiles changed")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
