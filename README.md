# Nfc-frog
> Kick-ass contactless credit card reader

With `nfc-frog` you can extract data from many contactless EMV credit cards. Also it supports mulitiple reading modes, so you can choose mode which suits you best.

Tested with: Visa, MasterCard, MIR (other cards should work too).

[![demo](https://asciinema.org/a/ZaCFZU2x9EN5gCAEvogPn4YGm.svg)](https://asciinema.org/a/ZaCFZU2x9EN5gCAEvogPn4YGm?autoplay=1)

# Installation

```bash
git clone https://github.com/cuamckuu/nfc-frog.git
cd nfc-frog
make
sudo ./nfc-frog

```

Project requires [libnfc(>= 1.7.1)](https://github.com/nfc-tools/libnfc#installation) and Pn532 as NFC reader

# Usage

Nfc-frog supports multiple modes for reading card data.

### GET PROCESSING DATA mode

This mode will call EMV command GET PROCESSING DATA and then it will read only files and record from command response.

In other words, it will 'emulate' POS terminal reading.

```bash
sudo ./nfc-frog GPO
```

### Bruteforce mode ('fast' and 'full')

Brutforce mode doesn't call GET PROCESSING OPTIONS to find files, it tries to use READ RECORD on many card files instead.

Both modes will iterate from SFI 1 to SFI 31 and for each existing file:

- Fast mode will read info from record 1 to record 16.
- Full mode will read info from record 1 to record 255.

```bash
sudo ./nfc-frog fast # Fast brute mode
sudo ./nfc-frog full # Full brute mode
```

### Disable logging

To get card data only, you should disable stderr output.

For example:

```bash
# This way
sudo ./nfc-frog fast 2>&-

# Or this way
sudo ./nfc-frog fast 2>/dev/null
```

### Parsing results

You can parse card data after reading by using one of many online EMV decoders. I personally prefer [this one](http://www.emvlab.org/tlvutils/)

Example with data from my MIR card:
![Parsed Data Example](https://pp.userapi.com/c854324/v854324574/acf4c/4t0KRDZhN8o.jpg)

# Why `nfc-frog`?

There are bunch of readers for NFC credit cards, but most of them just don't work or lacks some functionality. If there is no such problems, probably it's an android lib/app.

The main idea of `nfc-frog` was to get working tool with better availability for everyone. 

- Should work with still any EMV cards by calling SELECT command for finding card's applications;
- Works with raw data, without trying to parse EMV 'on fly', so there are no EMV-related errors;
- Full bruteforce mode won't skip any card files, even hidden from GET PROCESSING OPTIONS;
- You can use this tool with cheap pn532 ( about 5$ ), even if you don't have phone with NFC reader ( which price starts from 80$ ).
