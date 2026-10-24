# Basic build commands

Run these commands from the repository root:

```bash
cd /Users/dan/Developer/Entropy39/diceroll-bip39
```

## macOS simulator

Build only:

```bash
make simulator
```

Build and run:

```bash
make run
```

## Web simulator

Build:

```bash
make web
```

Run a local web server:

```bash
python3 -m http.server 8000 --directory build/web
```

Open <http://localhost:8000> in a browser. Press `Ctrl-C` in the
terminal to stop the server.

## STM32 firmware

```bash
make stm CUBE=/Users/dan/Developer/STM32CubeF4
```

## Waveshare firmware

```bash
make wave PICO_SDK_PATH=/Users/dan/Developer/pico-sdk
```

## Tests

```bash
make test
```

## Clean generated build files

```bash
make clean
```
