# 8-Bit Intercomputer Communication System

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![License](https://img.shields.io/badge/License-MIT-green)
![Platform](https://img.shields.io/badge/Platform-Embedded-red)
![Status](https://img.shields.io/badge/Status-Completed-brightgreen)
[![HTWK Leipzig](https://img.shields.io/badge/HTWK_Leipzig-Leipzig_University_of_Applied_Sciences-0077CC)](https://www.htwk-leipzig.de/)

A low-level communication system implementing 8-bit parallel data transfer between two ATmega microcontrollers. Developed as part of the Hardware Practical II module at Leipzig University of Applied Sciences.

## ✨ Key Features 

- **8-Bit Parallel Protocol** with hardware-level register control
- **Error Detection** through parity bit verification
- **Flow Control** using `ACK`/`DEN` handshaking
- **Data Framing** with `START`/`STOP` sequence markers
- **Collision Avoidance** through `SAME_SIGN`  repetition detection
- **Timeout Handling** with configurable retry intervals

## 🖥️ Hardware Requirements 

| Component          | Specification                          |
|--------------------|----------------------------------------|
| Microcontrollers   | 2× ATmega series chips                 |
| Connection         | 8-bit parallel bus + control lines     |
| Development Board  | B15F-compatible interface              |
| Power Supply       | Lab-grade 5V DC source                 |

## 📡 Communication Protocol 

### Flowchart

#### Sending-PC 

```mermaid
flowchart TD
    Y(START) --> A
    A(WHILE: not received STOP_SIGN) --> |true| B{IF: received START_SIGN}
    A --> |false|X(STOP)
    B --> |true|Z(lastBitset = START_SIGN)
    Z --> C(WHILE: less then 3 bitsets received)
    C --> |false|D{lastBitset == START_SIGN}
    D --> |true|C
    D --> |true|E{currentBitset == lastBitset}
    E --> |true|C
    E --> |false|F{lastBitset == SAME_SIGN}
    F --> |true|G(lastBitset = currentBitset)
    G --> C
    F --> |false|H(currentBitset gets saved)
    H --> I(lastBitset = currentBitset)
    I --> C
    C --> |false|J(Check parity of received bit vector)
    J --> K{IF: Is parity correct}
    K -->|false|L(Send DEN_SIGN)
    L --> B
    K --> |true|M(Send ACK_SIGN)
    M --> N(Remove parity from vector)
    N --> O(Save received vector)
    O --> A
```

#### Receiving-PC

``` mermaid
flowchart TD
    Y(START) --> A
    A[WHILE: Chars available for sending] -->|true| B(Send START_SIGN)
    B --> C[WHILE: Not all Bitsets of current char have been send]
    C --> |true| D[Send paket]
    C --> |false| C
    D --> E{IF: nextBitset == currentBitset}
    E --> |true|F(Send SAME_SIGN)
    E --> |false|C
    F --> C
    C --> |false|G{IF: ACK_SIGN received}
    G --> |true| H(Choose next char for sending)
    G --> |false| I(Choose the same char again)
    H --> A
    I --> A
    A --> |false|Z(STOP)

```

### Example Sequence

```plaintext
[Sender]                          [Receiver]
   |                                   |
   |───START_SIGN (0x08)──────────────>|
   |                                   |
   |───CHAR 1 - PACKET 1──────────────>| 
   |───CHAR 1 - PACKET 2──────────────>| 
   |───CHAR 1 - PACKET 3──────────────>| ✓ Parity correct
   |                                   |
   |<───────────────ACK (0x0B)─────────| ✓ 
   |                                   |
   |───CHAR 2 - PACKET 1──────────────>|
   |───CHAR 2 - PACKET 2──────────────>|
   |───CHAR 2 - PACKET 3--─-─-─-─-─-─->| ✗ Error in parity
   |                                   |
   |<───────────────DEN (0x09)─────────| ✗ 
   |                                   |
   |───CHAR 2 - PACKET 1 [RETRY]──────>| 
   |───CHAR 2 - PACKET 2 [RETRY]──────>|
   |───CHAR 2 - PACKET 3 [FIXED]──────>| ✓ Parity correct
   |                                   |
   |<───────────────ACK (0x0B)─────────| ✓
   |                                   |
   |───STOP_SIGN (0x0F)───────────────>|

```

### Control Codes

| Code | Hex | Purpose | 
| ------ | ------ | ------ | 
| REQ_SIG	| 0x0C | Transmission request |
| START_SIGN | 0x08 | Begin transmission | 
| SAME_SIG | 0x0A | Repeat previous character | 
| ACK_SIG	| 0x0B | Acknowledge - Parity is correct | 
| DEN_SIG | 0x09 | Denial - Parity is incorrect | 
| STOP_SIGN | 0x0F | End transmission | 

## 🔧 Technical Implementation 

### Data Packaging

- 3-Bit Chunking: 8-bit bytes split into three 3-bit packages
- Parity Generation: Even parity calculated across all bits, saved in the ninth bit
- Frame Assembly:

```cpp
vector<uint8_t> createBinaryVector(uint8_t num) {
    vector<uint8_t> binaryVector;
    for(int i=0; i<3; i++)
        binaryVector.push_back(7 & (num >> (i*3)));
    return binaryVector;
}
```

### Error Handling

- 40×500ms retry window for ACK
- Automatic DEN-triggered retransmission
- Parity verification through bit counting:

```cpp
bool checkParity(vector<uint8_t> binaryVector) {
    uint8_t parity = (binaryVector[2] >> 2);
    removeParity(binaryVector);
    return parity == (countOnes(binaryVector) % 2);
}
```

## 📜 License

This project is licensed under the MIT License - see the LICENSE file for details.

---

> Developed with ❤️ during my studies at HTWK Leipzig  
> 🚀 Feel free to contribute or fork this project!
