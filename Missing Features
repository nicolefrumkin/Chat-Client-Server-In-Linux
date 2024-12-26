### Missing Features in `server.c`
1. **Handling Multiple Clients:**
   - The current `server.c` code only handles a single client at a time. It needs to support multiple clients simultaneously.
   - Use `select()`, `poll()`, or `epoll()` to monitor multiple sockets for incoming messages.

2. **Client Identification:**
   - The server does not associate a name with the connected client as specified in the client command (`hw3client addr port name`). 
   - When a client connects, their name needs to be sent to the server and displayed along with their IP address in the format: `client name connected from address`.

3. **Broadcast Messages:**
   - Incoming messages from one client should be sent to all other connected clients, including the sender. This is missing.

4. **Whisper Messages:**
   - The server does not handle the whisper message syntax (`@friend msg`). It needs logic to parse messages starting with `@`, find the target client, and send the message only to them.

5. **Client Disconnection Handling:**
   - When a client disconnects, the server should detect this and print: `client name disconnected`.

6. **Prefix Message with Client Name:**
   - Messages relayed to clients need to include the sender's name in the format: `sourcename: message`.

---

### Missing Features in `client.c`
1. **Sending Client Name to Server:**
   - The client needs to send its name to the server upon connecting.

2. **Handling Whisper Messages:**
   - The client code needs logic to recognize the `@friend` syntax and send it to the server as a whisper message.

3. **Asynchronous Display of Incoming Messages:**
   - The client should continue displaying incoming messages while accepting input. This requires threading or non-blocking I/O.

4. **Proper `exit` Handling:**
   - When the user types `!exit`, the client needs to:
     - Send the `!exit` command to the server.
     - Print `client exiting`.
     - Close the connection and exit gracefully.

---

### Other Requirements
1. **Makefile:**
   - A `Makefile` is required to build `hw3server` and `hw3client` executables.

2. **Documentation:**
   - External documentation (PDF) describing the solution must be submitted.

3. **Comments:**
   - The source code should include comments explaining the implementation.

---

Would you like guidance on implementing any of these missing components?
