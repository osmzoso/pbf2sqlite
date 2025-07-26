/*
**
** Bits in the bitfield "permit":
**  Bit 0: foot
**  Bit 1: bike_gravel
**  Bit 2: bike_road
**  Bit 3: car
**  Bit 4: bike_oneway
**  Bit 5: car_oneway
**
*/
DROP TABLE IF EXISTS graph_permit;
CREATE TABLE graph_permit(
  key     TEXT,
  value   TEXT,
  set_bit INTEGER
);
/* Tags car -> set bits 001000 (hex 08) */
INSERT INTO graph_permit VALUES ('highway','motorway',       0x08);
INSERT INTO graph_permit VALUES ('highway','motorway_link',  0x08);
INSERT INTO graph_permit VALUES ('highway','trunk',          0x08);
INSERT INTO graph_permit VALUES ('highway','trunk_link',     0x08);
/* Tags car & bike -> set bits 001110 (hex 0e) */
INSERT INTO graph_permit VALUES ('highway','primary',        0x0e);
INSERT INTO graph_permit VALUES ('highway','primary_link',   0x0e);
INSERT INTO graph_permit VALUES ('highway','secondary',      0x0e);
INSERT INTO graph_permit VALUES ('highway','secondary_link', 0x0e);
INSERT INTO graph_permit VALUES ('highway','tertiary',       0x0e);
INSERT INTO graph_permit VALUES ('highway','tertiary_link',  0x0e);
INSERT INTO graph_permit VALUES ('highway','unclassified',   0x0e);
INSERT INTO graph_permit VALUES ('highway','residential',    0x0e);
/* Tags bike & foot -> set bits 000111 (hex 07) */
INSERT INTO graph_permit VALUES ('highway','residential',    0x07);
INSERT INTO graph_permit VALUES ('highway','living_street',  0x07);
INSERT INTO graph_permit VALUES ('highway','service',        0x07);
INSERT INTO graph_permit VALUES ('highway','cycleway',       0x07);  -- TODO
INSERT INTO graph_permit VALUES ('highway','track',          0x07);
INSERT INTO graph_permit VALUES ('highway','unclassified',   0x07);
INSERT INTO graph_permit VALUES ('highway','unclassified',   0x07);
INSERT INTO graph_permit VALUES ('bicycle','yes',            0x07);  -- TODO
INSERT INTO graph_permit VALUES ('bicycle','designated',     0x07);  -- TODO
/* Tags foot -> set bits 000001 (hex 01) */
INSERT INTO graph_permit VALUES ('highway','pedestrian',     0x01);
INSERT INTO graph_permit VALUES ('highway','track',          0x01);
INSERT INTO graph_permit VALUES ('highway','footway',        0x01);
INSERT INTO graph_permit VALUES ('highway','steps',          0x01);
INSERT INTO graph_permit VALUES ('highway','path',           0x01);
INSERT INTO graph_permit VALUES ('highway','construction',   0x01);
INSERT INTO graph_permit VALUES ('foot','yes',               0x01);
INSERT INTO graph_permit VALUES ('foot','designated',        0x01);
INSERT INTO graph_permit VALUES ('sidewalk','both',          0x01);
INSERT INTO graph_permit VALUES ('sidewalk:both','yes',      0x01);
INSERT INTO graph_permit VALUES ('sidewalk','right',         0x01);
INSERT INTO graph_permit VALUES ('sidewalk:right','yes',     0x01);
INSERT INTO graph_permit VALUES ('sidewalk','left',          0x01);
INSERT INTO graph_permit VALUES ('sidewalk:left','yes',      0x01);
INSERT INTO graph_permit VALUES ('sidewalk','yes',           0x01);
/* Tags oneway -> set bits 110000 (hex 30) */
INSERT INTO graph_permit VALUES ('oneway','yes',             0x30);
/*
0000 0x00
0001 0x01
0010 0x02
0011 0x03
0100 0x04
0101 0x05
0110 0x06
0111 0x07
1000 0x08
1001 0x09
1010 0x0a
1011 0x0b
1100 0x0c
1101 0x0d
1110 0x0e
1111 0x0f
*/
