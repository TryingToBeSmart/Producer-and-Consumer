# Producer-and-Consumer

This program uses a circular buffer to produce ints and add them to the buffer, then consume them. The program utilizes signals to wakeup a sleeping thread. The consumer goes to sleep when the buffer->count is 0 and then gets woke up after the producer adds to the buffer.

[![Producer and Consumer Video](./screenshots/Screenshot%202024-03-13%20001023.png)](https://www.loom.com/share/9e3e5c8abe0a4c1ba9ddeacebaa0956b?sid=431f6d99-f81b-4f31-acb0-e513d277f8cf)
