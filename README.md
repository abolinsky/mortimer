# mortimer
An interactive, structured timer for the terminal.

Get this:
![](./images/mortimer_0.1.0_demo.gif)
from a file containing this:
```
The best lecture ever
1. A terrific introduction (1m)
2. Our first topic (6m20s+)
3. Our second topic
    1. With subsections (2m30s+)
        1. And nested subsections (1m)
    2. This one is quite long (10m-)
        1. This is short (30s)
4. Almost done (15m-)
5. Wrap up (2m-)
```

## Features
- Intelligently redistributes remaining time based on how quickly you've completed past sections and how much time you need for future sections.
- Keyboard shortcuts for previous, next, pause, resume, quit.

## WIP
This is a work in progress. No releases have been made yet.

## TODO
- Robust feedback when input differs from expected format
- Handle both tabs and spaces in indented sections
