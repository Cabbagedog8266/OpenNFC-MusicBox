# KiCad Package — NFC_48x75_N3_Custom_YOLO_Mode

## Install (once)
1. Unzip this folder somewhere permanent.
2. **Preferences > Manage Footprint Libraries > +** — add this FOLDER (not the
   .kicad_mod file), library type "KiCad", nickname **AntennaGenerator**.
3. **Preferences > Manage Symbol Libraries > +** — add `NFC_48x75_N3_Custom_YOLO_Mode.kicad_sym`
   with the same nickname **AntennaGenerator**.

The symbol's Footprint property is written as
`AntennaGenerator:NFC_48x75_N3_Custom_YOLO_Mode`, so it only resolves under that nickname.

## Use
Place the symbol in your schematic, then **Update PCB from Schematic**. Pads bind
to symbol pins by NUMBER; the pin NAMES (FEED / GND / OUTER / INNER / ESCAPE …)
carry the meaning, because a footprint pad has no name field to put them in.

## Required to complete the board

The fabricated board is NOT finished when it arrives:

- **Pad 2 (inner terminal) is a plated thru-hole: route out on B.Cu**

Until that is done the coil is an open circuit, and no board house can do it for
you — it is not part of the artwork.

## What this footprint can and cannot express
- **Pads carry nets.** Every terminal is a numbered pad with a matching symbol
  pin, so each antenna lead is independently connectable.
- **Radiator and coil copper is footprint GRAPHICS and carries no net.** KiCad
  gives footprint graphics on copper layers no net at all — this is a limit of
  the footprint format, not an omission. Treat that copper as drawn copper: it is
  fabricated, but it does not appear in the ratsnest or in connectivity DRC.
- **Keep-out**, when you set one, ships as four rule areas ringing the antenna
  (no tracks, vias or copper pours). Your feed track still has to cross them.

## BOM
The BOM CSV lists LCSC part numbers for JLCPCB assembly.
