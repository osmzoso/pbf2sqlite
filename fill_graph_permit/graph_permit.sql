/*
**
** Bits in the bitfield "permit":
**  Bit 0: foot
**  Bit 1: bike
**  Bit 2: car
**  Bit 3: paved
**  Bit 4: bike_oneway
**  Bit 5: car_oneway
**  Bit 6: (not used)
**  Bit 7: (not used)
**
*/
DROP TABLE IF EXISTS graph_permit;
CREATE TABLE graph_permit(
  key     TEXT,
  value   TEXT,
  set_bit INTEGER,
  clear_bit INTEGER
);
/*
** Tags to set permit bits
*/
/* Tags foot -> set bits 00000001 (dec 1) */
INSERT INTO graph_permit VALUES ('highway','pedestrian',            1, 255);
INSERT INTO graph_permit VALUES ('highway','track',                 1, 255);
INSERT INTO graph_permit VALUES ('highway','footway',               1, 255);
INSERT INTO graph_permit VALUES ('highway','steps',                 1, 255);
INSERT INTO graph_permit VALUES ('highway','path',                  1, 255);
INSERT INTO graph_permit VALUES ('highway','construction',          1, 255);
INSERT INTO graph_permit VALUES ('foot','yes',                      1, 255);
INSERT INTO graph_permit VALUES ('foot','designated',               1, 255);
INSERT INTO graph_permit VALUES ('sidewalk','both',                 1, 255);
INSERT INTO graph_permit VALUES ('sidewalk:both','yes',             1, 255);
INSERT INTO graph_permit VALUES ('sidewalk','right',                1, 255);
INSERT INTO graph_permit VALUES ('sidewalk:right','yes',            1, 255);
INSERT INTO graph_permit VALUES ('sidewalk','left',                 1, 255);
INSERT INTO graph_permit VALUES ('sidewalk:left','yes',             1, 255);
INSERT INTO graph_permit VALUES ('sidewalk','yes',                  1, 255);
/* Tags foot & bike -> set bits 00000011 (dec 3) */
INSERT INTO graph_permit VALUES ('highway','residential',           3, 255);
INSERT INTO graph_permit VALUES ('highway','living_street',         3, 255);
INSERT INTO graph_permit VALUES ('highway','service',               3, 255);
INSERT INTO graph_permit VALUES ('highway','track',                 3, 255);
INSERT INTO graph_permit VALUES ('highway','unclassified',          3, 255);
/* Tags bike -> set bits 00000010 (dec 2) */
INSERT INTO graph_permit VALUES ('highway','cycleway',              2, 255);
INSERT INTO graph_permit VALUES ('bicycle','yes',                   2, 255);
INSERT INTO graph_permit VALUES ('bicycle','designated',            2, 255);
/* Tags bike & car -> set bits 00000110 (dec 6) */
INSERT INTO graph_permit VALUES ('highway','primary',               6, 255);
INSERT INTO graph_permit VALUES ('highway','primary_link',          6, 255);
INSERT INTO graph_permit VALUES ('highway','secondary',             6, 255);
INSERT INTO graph_permit VALUES ('highway','secondary_link',        6, 255);
INSERT INTO graph_permit VALUES ('highway','tertiary',              6, 255);
INSERT INTO graph_permit VALUES ('highway','tertiary_link',         6, 255);
INSERT INTO graph_permit VALUES ('highway','unclassified',          6, 255);
INSERT INTO graph_permit VALUES ('highway','residential',           6, 255);
/* Tags car -> set bits 00000100 (dec 4) */
INSERT INTO graph_permit VALUES ('highway','motorway',              4, 255);
INSERT INTO graph_permit VALUES ('highway','motorway_link',         4, 255);
INSERT INTO graph_permit VALUES ('highway','trunk',                 4, 255);
INSERT INTO graph_permit VALUES ('highway','trunk_link',            4, 255);
/* Tags paved -> set bits 00001000 (dec 8) */
INSERT INTO graph_permit VALUES ('surface','asphalt',               8, 255);
INSERT INTO graph_permit VALUES ('surface','sett',                  8, 255);
INSERT INTO graph_permit VALUES ('surface','paving_stones',         8, 255);
/* Tags oneway -> set bits 00110000 (dec 48) */
INSERT INTO graph_permit VALUES ('oneway','yes',                   48, 255);
/*
** Tags to clear permit bits
*/
/* Tags no foot -> clear bits 11111110 (dec 254) */
INSERT INTO graph_permit VALUES ('sidewalk','separate',             0, 254);
INSERT INTO graph_permit VALUES ('foot','use_sidepath',             0, 254);
INSERT INTO graph_permit VALUES ('access','no',                     0, 254);
/* Tags no bike -> clear bits 11111101 (dec 253) */
INSERT INTO graph_permit VALUES ('cycleway','separate',             0, 253);
INSERT INTO graph_permit VALUES ('cycleway:both','separate',        0, 253);
INSERT INTO graph_permit VALUES ('cycleway:right','separate',       0, 253);
INSERT INTO graph_permit VALUES ('cycleway:left','separate',        0, 253);
INSERT INTO graph_permit VALUES ('bicycle','use_sidepath',          0, 253);
INSERT INTO graph_permit VALUES ('access','no',                     0, 253);
/* Tags no oneway -> clear bits 11101111 (dec 239) */
INSERT INTO graph_permit VALUES ('oneway:bicycle','no',             0, 239);

CREATE INDEX graph_permit__key_value ON graph_permit (key, value);
