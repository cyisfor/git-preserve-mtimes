add_find
SELECT id FROM entries WHERE name = ? AND parent = ? AND (NOT isdir IS ? OR modified != ? OR modifiedns != ?);
add_insert
INSERT INTO entries (name, parent, isdir, modified, modifiedns) VALUES (?,?,?,?,?);
has_seen
SELECT id FROM saw where id = ?;
see_entry
INSERT INTO saw (id) VALUES (?);
