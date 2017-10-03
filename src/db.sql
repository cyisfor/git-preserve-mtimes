CREATE TABLE IF NOT EXISTS entries (
			 id INTEGER PRIMARY KEY,
			 name TEXT NOT NULL,
			 -- isdir is just for speed, so to save one parent/child lookup
			 isdir BOOLEAN NOT NULL DEFAULT FALSE,
			 -- root entry is its own parent, so stop if entry is its own parent
			 -- don't need a next, since we get the children via parent lookup
			 parent INTEGER REFERENCES entries(id) NOT NULL DEFAULT 0,
			 modified INTEGER NOT NULL,
			 modifiedns INTEGER NOT NULL,
			 UNIQUE(name,parent));

-- to find children for this parent
CREATE INDEX IF NOT EXISTS parent_lookup ON entries(parent);

-- be sure to do a POST order traversal, first the children, then the entry
-- otherwise you set a directory's mtime, then set mtimes inside it,
-- which might update its modification, and will update its atime
-- I guess not b/c who cares about atime

-- we always need to have a starting point to restore from...

INSERT INTO entries (id,name,isdir,parent,modified,modifiedns)
VALUES
(0,"<root>", 1, 0, 0, 0);


CREATE TEMPORARY TABLE saw (
id INTEGER PRIMARY KEY REFERENCES entries(id));
