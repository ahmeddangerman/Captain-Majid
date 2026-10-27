"""Bounded CPU memory for private PRG traces; no PPU/APU/frame emulation."""
class ReferenceCartridge:
    def __init__(self, prg):
        if len(prg) != 32 * 8192:
            raise ValueError('Expected 32 PRG banks')
        self.prg = prg
        self.ram = bytearray(32768)
        self.registers = [0] * 8
        self.registers[6:8] = [26, 27]
        self.select = 0
        self.bank_writes = []

    def bank_at(self, address):
        mode = bool(self.select & 64)
        banks = [30 if mode else self.registers[6], self.registers[7],
                 self.registers[6] if mode else 30, 31]
        return banks[(address - 0x8000) // 8192] & 31

    def __getitem__(self, address):
        if isinstance(address, slice):
            return [self[i] for i in range(*address.indices(65536))]
        if address < 0x2000:
            return self.ram[address & 2047]
        if address < 0x6000:
            raise RuntimeError(f'Hardware read outside bounded trace: ${address:04X}')
        if address < 0x8000:
            return self.ram[address]
        return self.prg[self.bank_at(address) * 8192 + (address & 8191)]

    def __setitem__(self, address, value):
        if isinstance(address, slice):
            indices = range(*address.indices(65536))
            if len(indices) != len(value):
                raise ValueError('Slice must preserve memory length')
            for i, byte in zip(indices, value):
                self[i] = byte
        elif address < 0x2000:
            self.ram[address & 2047] = value
        elif address < 0x6000:
            raise RuntimeError(f'Hardware write outside bounded trace: ${address:04X}')
        elif address < 0x8000:
            self.ram[address] = value
        elif address < 0xA000:
            if address & 1:
                register = self.select & 7
                if register not in (6, 7):
                    raise RuntimeError('CHR bank writes outside bounded trace')
                self.registers[register] = value
                self.bank_writes.append((register, value))
            else:
                self.select = value
        else:
            raise RuntimeError(f'Unsupported mapper write: ${address:04X}')
