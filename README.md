# SearchInform

Client–server file transfer over **Windows shared memory**.

## How it works

- The server creates named shared memory (`Global\SharedMemoryExample`, 256 KB)
  and a global mutex.
- A transfer starts with a header: `isWriting` flag, payload size and file name.
- The mutex serializes access, so the client and server never read a partially
  written buffer.

## Stack

C++, Win32 API (`CreateFileMapping`, `MapViewOfFile`, `Mutex`), Visual Studio.

## Build & run

1. Open `SearchInform.sln` in Visual Studio and build `Server` and `Client`.
2. Run **as administrator** — the names live in the `Global\` namespace.

```
Server.exe
Client.exe
```
