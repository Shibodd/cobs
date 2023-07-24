# cobs
Logging library for our real-time STM board.
- Physical layer: UART to a Raspberry PI.
- Transport layer: encode with COBS, add a CRC16 and terminate each frame with a zero byte.
- Logging: Compact message (message id, tick, sequence id, fields), STM board has no knowledge of how the data will be actually formatted. Logging functions write to a ring buffer. Data will be actually transmitted at a later point in time using DMA. The python script on the other side will actually format and store the data.
