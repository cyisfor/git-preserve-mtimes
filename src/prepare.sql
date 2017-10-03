add_find
SELECT id FROM entries WHERE name = ? AND parent = ?)
add_insert
INSERT INTO entries (name, parent, modified, modifiedns) VALUES (?,?,?,?)")
