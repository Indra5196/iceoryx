# Git Hooks for iceoryx

When developing on iceoryx, git-hooks are doing some sanity checks and can execute linters/formatters when you commit code to iceoryx.
In the current state the git hooks do the following job:

- checking that commit messages follow the format `[iox-#<Github issue number>]` and auto-format them if necessary
- check that the commit messages subject and body follow six of the [seven rules of a great commit message](https://chris.beams.io/posts/git-commit/)
- remove trailing whitespace from all modified files
- run `clang-format` on all modified files

For running `clang-format` it checks if the version 10 is used, otherwise it will print a warning,
you can install it by using the following command:

```bash
sudo apt install clang-format-10
```

## Installation

The hooks are active when you add the `git-hooks` directory as hooks folder to your local project git config:

```bash
cd iceoryx
git config core.hooksPath tools/git-hooks/
```

With that you will also receive the updates of the git hooks in the future.
We recommend doing this in every new clone you did on iceoryx.

## Seven Rules for a good commit message

1. [Separate subject from body with a blank line](https://chris.beams.io/posts/git-commit/#separate)
2. [Limit the subject line to 50 characters](https://chris.beams.io/posts/git-commit/#limit-50) (excluding GitHub issue number)
3. [Capitalize the subject line](https://chris.beams.io/posts/git-commit/#capitalize)
4. [Do not end the subject line with a period](https://chris.beams.io/posts/git-commit/#end)
5. [Use the imperative mood in the subject line](https://chris.beams.io/posts/git-commit/#imperative)
6. [Wrap the body at 72 characters](https://chris.beams.io/posts/git-commit/#wrap-72)
7. Use the body to explain what and why vs. how (not checked in hooks)

## Open topics

- clang-tidy check
- ...
