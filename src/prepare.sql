add_find
SELECT id FROM entries WHERE name = ? AND parent = ? AND (NOT isdir IS ? OR modified != ? OR modifiedns != ?);
update
UPDATE entries SET isdir = ?1, modified = ?2, modifiedns = ?3
where isdir IS NOT ?1 OR modified != ?2 OR modifiedns != ?3
AND id = ?4
add_insert
INSERT INTO entries (name, parent, isdir, modified, modifiedns) VALUES (?,?,?,?,?);
has_seen
SELECT id FROM saw where id = ?;
see_entry
INSERT INTO saw (id) VALUES (?);
