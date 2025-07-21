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
DROP TABLE IF EXISTS permit_def;
CREATE TABLE permit_def(
  key     TEXT,
  value   TEXT,
  set_bit INTEGER
);
/* Tags car -> set bits 001000 (hex 08) */
INSERT INTO permit_def VALUES ('highway','motorway',       0x08);
INSERT INTO permit_def VALUES ('highway','motorway_link',  0x08);
INSERT INTO permit_def VALUES ('highway','trunk',          0x08);
INSERT INTO permit_def VALUES ('highway','trunk_link',     0x08);
/* Tags car & bike -> set bits 001110 (hex 0e) */
INSERT INTO permit_def VALUES ('highway','primary',        0x0e);
INSERT INTO permit_def VALUES ('highway','primary_link',   0x0e);
INSERT INTO permit_def VALUES ('highway','secondary',      0x0e);
INSERT INTO permit_def VALUES ('highway','secondary_link', 0x0e);
INSERT INTO permit_def VALUES ('highway','tertiary',       0x0e);
INSERT INTO permit_def VALUES ('highway','tertiary_link',  0x0e);
INSERT INTO permit_def VALUES ('highway','unclassified',   0x0e);
INSERT INTO permit_def VALUES ('highway','residential',    0x0e);
/* Tags bike & foot -> set bits 000111 (hex 07) */
INSERT INTO permit_def VALUES ('highway','residential',    0x07);
INSERT INTO permit_def VALUES ('highway','living_street',  0x07);
INSERT INTO permit_def VALUES ('highway','service',        0x07);
INSERT INTO permit_def VALUES ('highway','cycleway',       0x07);  -- TODO
INSERT INTO permit_def VALUES ('highway','track',          0x07);
INSERT INTO permit_def VALUES ('highway','unclassified',   0x07);
INSERT INTO permit_def VALUES ('highway','unclassified',   0x07);
INSERT INTO permit_def VALUES ('bicycle','yes',            0x07);  -- TODO
INSERT INTO permit_def VALUES ('bicycle','designated',     0x07);  -- TODO
/* Tags foot -> set bits 000001 (hex 01) */
INSERT INTO permit_def VALUES ('highway','pedestrian',     0x01);
INSERT INTO permit_def VALUES ('highway','track',          0x01);
INSERT INTO permit_def VALUES ('highway','footway',        0x01);
INSERT INTO permit_def VALUES ('highway','steps',          0x01);
INSERT INTO permit_def VALUES ('highway','path',           0x01);
INSERT INTO permit_def VALUES ('highway','construction',   0x01);
INSERT INTO permit_def VALUES ('foot','yes',               0x01);
INSERT INTO permit_def VALUES ('foot','designated',        0x01);
INSERT INTO permit_def VALUES ('sidewalk','both',          0x01);
INSERT INTO permit_def VALUES ('sidewalk:both','yes',      0x01);
INSERT INTO permit_def VALUES ('sidewalk','right',         0x01);
INSERT INTO permit_def VALUES ('sidewalk:right','yes',     0x01);
INSERT INTO permit_def VALUES ('sidewalk','left',          0x01);
INSERT INTO permit_def VALUES ('sidewalk:left','yes',      0x01);
INSERT INTO permit_def VALUES ('sidewalk','yes',           0x01);
