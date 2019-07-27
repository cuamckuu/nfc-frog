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
