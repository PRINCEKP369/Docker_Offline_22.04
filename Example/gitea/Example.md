Great practical example! Here's how your `.gitea/workflows/ci.yml` would look:

---

## Assumed Repo Structure

```
your-repo/
├── .gitea/
│   └── workflows/
│       └── ci.yml
├── sum.c
└── factorial.py
```

---

## ci.yml

```yaml
name: CI Pipeline
on: [push, pull_request]

jobs:

  build-and-test-c:
    runs-on: self-hosted
    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Install GCC
        run: sudo apt-get install -y gcc

      - name: Compile C program
        run: gcc sum.c -o sum

      - name: Run C program
        run: ./sum

  test-python:
    runs-on: self-hosted
    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Install Python
        run: sudo apt-get install -y python3

      - name: Run Python program
        run: python3 factorial.py
```

---

## What Each Job Does

### Job 1 — `build-and-test-c`
| Step | What happens |
|---|---|
| Checkout | Pulls your repo code onto the runner |
| Install GCC | Installs C compiler if not present |
| Compile | Runs `gcc sum.c -o sum` — fails here if code has errors |
| Run | Executes `./sum` — fails here if program crashes |

### Job 2 — `test-python`
| Step | What happens |
|---|---|
| Checkout | Pulls your repo code onto the runner |
| Install Python | Installs python3 if not present |
| Run | Executes `factorial.py` — fails if there's a syntax or runtime error |

---

## Both Jobs Run in Parallel

```
Push to repo
     ↓
  CI triggers
     ↙        ↘
C job         Python job
(compile+run)  (run)
     ↘        ↙
   Both must pass ✅
```

---

## If You Want Python Job to Run Only After C Job Passes

Add `needs`:

```yaml
  test-python:
    runs-on: self-hosted
    needs: build-and-test-c    # ← waits for C job to finish first
    steps:
      ...
```

---

## Pro Tip — Since It's Self-Hosted

GCC and Python3 are likely **already installed** on your Ubuntu 22.04. So the install steps won't reinstall — `apt-get` will just skip them with `already the newest version`. You can even remove those steps if you're sure they're installed:

```bash
# Check on your server
gcc --version
python3 --version
```

If both return versions, your workflow will work as-is! Want me to add actual test assertions too — like checking the output of `sum.c` equals an expected value?
