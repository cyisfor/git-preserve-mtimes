add_find
SELECT id FROM entries WHERE name = ? AND parent = ?;
add_insert
INSERT INTO entries (name, parent, isdir, modified, modifiedns) VALUES (?,?,?,?,?);
has_seen
SELECT id FROM has where id = ?;
see_entry
INSERT INTO has (id) VALUES (?);
