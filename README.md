Two hooks, to preserve file modification times using git.

Normally when you say `git checkout .` all your file modification times are wiped and set to the current time. git likes to do that in a lot of cases, resetting, filter-branch, rebasing, it always sets the modification time to the current time, even if the file was *actually* modified 3 weeks ago.

Even when you checkout a file from 3 weeks ago, git doesn’t even use the *commit time* but sets file modification to the time of checkout.

So... say for instance if you wish for people to know which of your blog entries was last modified... you’re hosed.

Thus, this tool. Compile, and run hook-store in a pre-commit hook. It will create a (uncompressed binary) file called .git_times, then add it to your repository right before the commit. That file will contain the current file modification times for every file in the tree
