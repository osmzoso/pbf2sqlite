/*
**
** Bits in the bitfield "permit":
**  Bit 0: foot
**  Bit 1: bike_gravel
**  Bit 2: bike_road
**  Bit 3: car
**  Bit 4: bike_oneway
**  Bit 5: car_oneway
**  Bit 6: (not used)
**  Bit 7: (not used)
**
** TODO: New field to dectect surface=asphalt
**       Tags: 'surface=asphalt', 'surface=sett',  'surface=paving_stones'
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
** key values to set bits
*/
/* Tags car -> set bits 00001000 (hex 08) */
INSERT INTO graph_permit VALUES ('highway','motorway',       0x08, 255);
INSERT INTO graph_permit VALUES ('highway','motorway_link',  0x08, 255);
INSERT INTO graph_permit VALUES ('highway','trunk',          0x08, 255);
INSERT INTO graph_permit VALUES ('highway','trunk_link',     0x08, 255);
/* Tags car & bike -> set bits 00001110 (hex 0e) */
INSERT INTO graph_permit VALUES ('highway','primary',        0x0e, 255);
INSERT INTO graph_permit VALUES ('highway','primary_link',   0x0e, 255);
INSERT INTO graph_permit VALUES ('highway','secondary',      0x0e, 255);
INSERT INTO graph_permit VALUES ('highway','secondary_link', 0x0e, 255);
INSERT INTO graph_permit VALUES ('highway','tertiary',       0x0e, 255);
INSERT INTO graph_permit VALUES ('highway','tertiary_link',  0x0e, 255);
INSERT INTO graph_permit VALUES ('highway','unclassified',   0x0e, 255);
INSERT INTO graph_permit VALUES ('highway','residential',    0x0e, 255);
/* Tags bike & foot -> set bits 00000111 (hex 07) */
INSERT INTO graph_permit VALUES ('highway','residential',    0x07, 255);
INSERT INTO graph_permit VALUES ('highway','living_street',  0x07, 255);
INSERT INTO graph_permit VALUES ('highway','service',        0x07, 255);
INSERT INTO graph_permit VALUES ('highway','cycleway',       0x07, 255);  -- TODO
INSERT INTO graph_permit VALUES ('highway','track',          0x07, 255);
INSERT INTO graph_permit VALUES ('highway','unclassified',   0x07, 255);
INSERT INTO graph_permit VALUES ('highway','unclassified',   0x07, 255);
INSERT INTO graph_permit VALUES ('bicycle','yes',            0x07, 255);  -- TODO
INSERT INTO graph_permit VALUES ('bicycle','designated',     0x07, 255);  -- TODO
/* Tags foot -> set bits 00000001 (dec 1) */
INSERT INTO graph_permit VALUES ('highway','pedestrian',     1, 255);
INSERT INTO graph_permit VALUES ('highway','track',          1, 255);
INSERT INTO graph_permit VALUES ('highway','footway',        1, 255);
INSERT INTO graph_permit VALUES ('highway','steps',          1, 255);
INSERT INTO graph_permit VALUES ('highway','path',           1, 255);
INSERT INTO graph_permit VALUES ('highway','construction',   1, 255);
INSERT INTO graph_permit VALUES ('foot','yes',               1, 255);
INSERT INTO graph_permit VALUES ('foot','designated',        1, 255);
INSERT INTO graph_permit VALUES ('sidewalk','both',          1, 255);
INSERT INTO graph_permit VALUES ('sidewalk:both','yes',      1, 255);
INSERT INTO graph_permit VALUES ('sidewalk','right',         1, 255);
INSERT INTO graph_permit VALUES ('sidewalk:right','yes',     1, 255);
INSERT INTO graph_permit VALUES ('sidewalk','left',          1, 255);
INSERT INTO graph_permit VALUES ('sidewalk:left','yes',      1, 255);
INSERT INTO graph_permit VALUES ('sidewalk','yes',           1, 255);
/* Tags oneway -> set bits 00110000 (hex 30) */
INSERT INTO graph_permit VALUES ('oneway','yes',             0x30, 255);

/*
** key values to clear bits
*/
/* Tags no foot -> clear bits 11111110 (dec 254) */
INSERT INTO graph_permit VALUES ('sidewalk','separate',             0, 254);
INSERT INTO graph_permit VALUES ('foot','use_sidepath',             0, 254);
INSERT INTO graph_permit VALUES ('access','no',                     0, 254);
/* Tags no bike -> clear bits 11111001 (dec 249) */
INSERT INTO graph_permit VALUES ('cycleway','separate',             0, 249);
INSERT INTO graph_permit VALUES ('cycleway:both','separate',        0, 249);
INSERT INTO graph_permit VALUES ('cycleway:right','separate',       0, 249);
INSERT INTO graph_permit VALUES ('cycleway:left','separate',        0, 249);
INSERT INTO graph_permit VALUES ('bicycle','use_sidepath',          0, 249);
INSERT INTO graph_permit VALUES ('access','no',                     0, 249);
/* Tags no oneway -> clear bits 11101111 (dec 239) */
INSERT INTO graph_permit VALUES ('oneway:bicycle','no',             0, 239);


CREATE INDEX graph_permit__key_value ON graph_permit (key, value);
