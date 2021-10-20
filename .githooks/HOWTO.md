# How to use git hooks for this repository:

* Set `.githooks` directory as root directory for git hooks:

```bash
git config --local core.hooksPath ./.githooks
```

or

* Copy `pre-commit` file to `.git/hooks`:

```bash
cp ./.githooks/pre-commit ./.git/hooks/
```

# Available git hooks:
* pre-commit: do auto clang-format and linebreak at the end of each file in git diff before every commit
