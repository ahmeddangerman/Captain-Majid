"""Explicit private RAM to current saved-flight snapshot for comparison tools."""
from check_attack_clock_original import checkpoint


def flight_checkpoint(ram,phase,loose=True):
    b=bytearray(checkpoint(ram,phase))
    b[339:344]=bytes([1,ram[0x62B],ram[0x638],ram[0x61A],int(loose)])
    if phase>=47:
        b[324:330]=bytes(6)
        velocity=bytes(4) if phase==50 else ram[0x639:0x63D]
        b[344:355]=bytes([1])+ram[0x634:0x638]+velocity+bytes([ram[0x62C],ram[0x61B]])
    return bytes(b)
