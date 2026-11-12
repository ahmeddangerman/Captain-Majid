#!/usr/bin/env python3
"""Export private Japanese development data to a local CMJSHOTK/L/M/N/O/P/Q pack."""
import argparse
import hashlib
import struct
from pathlib import Path
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256


def word(memory, address):
    return memory[address] | memory[address + 1] << 8


def no_variant_zero_shot_option(memory, pointer):
    """Conservatively gate ordinary shots when variant-zero special entries exist."""
    if not pointer:
        return True
    if not 0x8000 <= pointer < 0xc000:
        return False
    possible = False
    for _ in range(128):
        if not 0x8000 <= pointer < 0xc000:
            raise ValueError('Shot option list leaves mapped stats banks')
        encoded = memory[pointer]; pointer += 1
        if encoded & 3 == 3:
            return not possible
        possible |= encoded & 3 == 0
        if encoded >> 2 in (8, 9, 10, 17, 19):
            if pointer >= 0xc000: raise ValueError('Shot option operand leaves mapped stats banks')
            pointer += 1
    raise ValueError('Shot option list lacks a bounded terminator')


def selected_team_bytes(stats, team_id):
    """Read a caller-selected, original team record; selection remains external."""
    if team_id not in (3, 4, 5, 6):
        raise ValueError('Only traced team IDs 3..6 are supported')
    pointer = word(stats, 0xBAB2 + 2 * (team_id - 3))
    size = max(stats[0x818E:0x8199]) + 1
    if not (0x8000 <= pointer and pointer + size <= 0xC000):
        raise ValueError('Selected team record is outside the mapped team banks')
    return bytes(stats[pointer:pointer + size])


def export_data(rom, team_id=None, include_field_ai=False, include_field_ai_pass=False, include_pass_flight=False, include_pass_race=False, include_keeper_ai=False, include_kickoff=False, include_cell_ai=False, include_cell_shots=False, include_keeper_menu=False, include_pass_defense=False, include_first_match=False, include_generic_pass_defense=False, include_pass_action2=False):
    if len(rom) != 393232 or rom[:4] != b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        raise ValueError('ROM does not match verified Japanese reference')
    prg = rom[16:262160]
    include_generic_pass_defense = include_generic_pass_defense or include_pass_action2
    include_first_match = include_first_match or include_generic_pass_defense
    if include_first_match and team_id!=3:
        raise ValueError('The traced first fixture requires opponent team ID 3')
    stats = ReferenceCartridge(prg)
    stats.registers[6:8] = [28, 29]
    gameplay = ReferenceCartridge(prg)
    include_pass_defense = include_pass_defense or include_first_match
    include_keeper_menu = include_keeper_menu or include_pass_defense
    include_cell_shots = include_cell_shots or include_keeper_menu
    include_cell_ai = include_cell_ai or include_cell_shots
    include_kickoff = include_kickoff or include_cell_ai
    include_keeper_ai = include_keeper_ai or include_kickoff
    if include_keeper_ai and team_id not in (3, 4, 5, 6):
        raise ValueError('Keeper AI requires an explicit traced team ID 3..6')
    include_pass_race = include_pass_race or include_keeper_ai
    include_pass_flight = include_pass_flight or include_pass_race
    include_field_ai_pass = include_field_ai_pass or include_pass_flight
    include_field_ai = include_field_ai or include_field_ai_pass
    data = bytearray((b'CMJSHOTa' if include_pass_action2 else b'CMJSHOTZ' if include_generic_pass_defense else b'CMJSHOTY' if include_first_match else b'CMJSHOTX' if include_pass_defense else b'CMJSHOTT' if include_keeper_menu else b'CMJSHOTS' if include_cell_shots else b'CMJSHOTR' if include_cell_ai else b'CMJSHOTQ' if include_kickoff else b'CMJSHOTP' if include_keeper_ai else b'CMJSHOTO' if include_pass_race else b'CMJSHOTN' if include_pass_flight else b'CMJSHOTM' if include_field_ai_pass else b'CMJSHOTL' if include_field_ai else b'CMJSHOTK') + hashlib.sha256(rom[16:]).digest())

    def vector(values):
        data.extend(struct.pack('<H', len(values)))
        data.extend(values)

    def move_lists(pointers):
        for start, end in zip(pointers, pointers[1:]):
            size = end - start
            if not (0x8000 <= start <= end <= 0xC000) or size % 4 or size > 256:
                raise ValueError('Unsupported move table bounds')
            data.append(size // 4)
            data.extend(stats[start:end])

    data.extend(stats[0x95D6 + 4 * i] for i in range(35))
    vector([stats[0x9662 + 12 * i] for i in range(221)])
    data.extend(stats[0x818E:0x8199])
    vector([] if team_id is None else selected_team_bytes(stats, team_id))
    data.extend(struct.pack('<H', 256))
    for profile in range(256):
        data.extend(stats[0x9FCE + 24 * profile:0x9FCE + 24 * profile + 31])
    data.extend(struct.pack('<H', 256))
    for profile in range(256):
        data.extend(stats[0xAE86 + 8 * profile:0xAE86 + 8 * profile + 8])
    curve = stats[0x9E4E:0x9F0E]
    data.extend(stats[0x8206:0x8215])
    move_lists([word(stats, 0x9460 + 2 * i) for i in range(6)])
    data.extend(curve)
    data.extend(stats[0x824C:0x825B])
    move_lists([word(stats, 0x9554 + 2 * i) for i in range(5)] + [0x959E])
    data.extend(curve)
    data.extend(stats[0x82C0 + i] for i in (0, 1, 2, 5))
    move_lists([word(stats, 0x959E + 2 * i) for i in range(5)])
    data.extend(curve)
    data.extend(gameplay[0x83D7:0x83DC])
    data.extend(gameplay[0x83DD:0x83E2])
    vector(gameplay[0x83E1:0x84E1])
    data.extend(struct.pack('<H', 128))
    variation = word(gameplay, 0x8D95)
    data.extend(gameplay[variation:variation + 256])
    data.extend(gameplay[0x83F1:0x83F5])
    # Pointer table starts at8D93; keeper selector3 indexes its fourth pointer.
    keeper_variation = word(gameplay, 0x8D93 + 6)
    data.extend(gameplay[keeper_variation:keeper_variation + 2])
    data.extend(gameplay[0x86B9:0x86BD])
    data.extend(gameplay[0x86B1:0x86B9])
    data.extend(gameplay[0xDDCB:0xDDD9])
    data.extend(gameplay[0xDDD9:0xDDFD])
    # Only the two supported action0 flight variants; do not export adjacent
    # instruction bytes as hypothetical action1..4 eligibility/threshold rows.
    flight_actions = gameplay[0x888B:0x888D]
    data.extend(flight_actions)
    data.extend(gameplay[0x88EB + action] for action in flight_actions)
    flight_variation = word(gameplay, 0x8D93 + 10)
    for action in flight_actions:
        data.extend(gameplay[flight_variation + 2 * action:flight_variation + 2 * action + 2])
    for action in flight_actions:
        data.extend(gameplay[0x828C + 4 * action:0x828C + 4 * action + 4])
    data.extend(gameplay[0xFACC:0xFB4C])
    data.extend(gameplay[0xE8ED:0xE8F5])
    for player_type in range(35):
        data.extend(stats[0x95D6 + 4 * player_type + 1:0x95D6 + 4 * player_type + 4])
    data.extend(struct.pack('<H', 221))
    for alternate in range(221):
        data.extend(stats[0x9662 + 12 * alternate + 1:0x9662 + 12 * alternate + 4])
    data.extend(struct.pack('<H', 256))
    for profile in range(256):
        data.extend(stats[0xAFAE + 12 * profile + 4:0xAFAE + 12 * profile + 8])
    data.extend(gameplay[0xA1DC:0xA1EB])
    for table, count, lower, upper in ((0xA6AD, 4, 0xA6B5, 0xAB65),
                                      (0xAB65, 12, 0xAB7D, 0xB98D)):
        for index in range(count):
            start = word(gameplay, table + 2 * index)
            if not lower <= start <= upper - 300:
                raise ValueError('Unsupported field formation template bounds')
            data.extend(gameplay[start:start + 300])
    data.extend(gameplay[0x90E6:0x90F4])
    data.extend(gameplay[0x90F4:0x9102])
    data.extend(gameplay[0x9102:0x9109])
    progression = ReferenceCartridge(prg)
    progression.registers[6:8] = [0, 1]
    data.extend(progression[0xB9D3:0xB9D6])
    data.extend(progression[0xB9D6:0xBA1C])
    data.extend(progression[0xBA1C:0xBA4C])
    data.extend(progression[0xBA4C:0xBA90])
    for profile in range(256):
        data.extend(stats[0xAFAE + 12 * profile + 2:0xAFAE + 12 * profile + 4])
    for profile in range(256):
        data.extend(stats[0xAFAE + 12 * profile:0xAFAE + 12 * profile + 2])
    data.extend(stats[0x9F0E:0x9FCE])
    # Human action-menu directions, icons, titles and backgrounds from fixed bank30.
    menu = ReferenceCartridge(prg)
    data.extend(menu[0xD6E8:0xD700])
    data.extend(menu[0xD6DE:0xD6E8])
    data.extend(menu[0xD706:0xD70C])
    data.extend(menu[0xD700:0xD706])
    data.extend(menu[0xD552:0xD555])
    data.extend(menu[0xD555:0xD561])
    for action in range(7):
        pointer=word(menu,0xD52B+2*action)
        if not 0xD539 <= pointer < 0xD548:
            raise ValueError('Unsupported defensive icon pointer')
        data.append(menu[pointer])
    # Original8D65..8E1A uses raw player type, not resolved skill profile.
    groups=word(stats,0x8E1B)
    if not 0x8E1D<=groups<=0x901B or (groups-0x8E1B)%2:
        raise ValueError('Unsupported defensive option type table')
    count=(groups-0x8E1B)//2
    data.extend(struct.pack('<H',count))
    for player_type in range(count):
        group=word(stats,0x8E1B+2*player_type)
        if not groups<=group<=0x934D-14:
            raise ValueError('Unsupported defensive option group')
        pointer=word(stats,group+12)  # action2 is group slot(action+4).
        data.append(int(bool(pointer)))
        if pointer:
            if not 0x934D<=pointer<0x9460:
                raise ValueError('Unsupported defensive option rule pointer')
            value=stats[pointer];conditional=not bool(value&128)
            if conditional and pointer+1>=0x9460:
                raise ValueError('Truncated defensive teammate condition')
            data.extend((value&127,int(conditional),stats[pointer+1] if conditional else 0))
    # Count-one special chooser cursor: tile and horizontal position.
    data.extend((menu[0xD003],menu[0xD01B]))
    data.append(menu[0xD00B])  # Count-one chooser cursor Y origin.
    # The action2 list must have a second icon for its supported special subtype.
    action2=word(menu,0xD52B+2*2)
    next_action=word(menu,0xD52B+2*3)
    if not 0xD539<=action2<action2+1<next_action<=0xD548:
        raise ValueError('Unsupported action2 special icon bounds')
    data.append(menu[action2+1])
    categories=[stats[0x9662+12*i+6] for i in range((0x9E4E-0x9662)//12)]
    vector(categories)
    data.extend(stats[0x8B9E:0x8BBE])
    rows=max(categories)+1
    if rows>64 or 0xB12E+48*rows>0xC000:raise ValueError('Unsupported AI row bounds')
    data.extend(struct.pack('<H',rows))
    data.extend(stats[0xB12E:0xB12E+48*rows])
    recipients=[stats[0x9662+12*i+10] for i in range((0x9E4E-0x9662)//12)]
    vector(recipients)
    recipient_rows=max(recipients)+1
    if recipient_rows>64 or 0xB72E+8*recipient_rows>0xC000:raise ValueError('Unsupported AI recipient bounds')
    data.extend(struct.pack('<H',recipient_rows))
    data.extend(stats[0xB72E:0xB72E+8*recipient_rows])
    data.extend(stats[0x8206+18:0x8206+21])
    start=word(stats,0x9460+12);end=word(stats,0x9460+14)
    if not 0x9474<=start<end<=0x9554 or end-start!=8:raise ValueError('Unsupported area attack6 move bounds')
    data.append(2);data.extend(stats[start:end])
    data.extend(gameplay[0x8A63:0x8A6A])
    data.extend(gameplay[0x8A6A:0x8A6F])
    area_variation=word(gameplay,0x8D93+16)
    data.extend(gameplay[area_variation:area_variation+20])
    data.extend(gameplay[0x8AAC:0x8AB0])
    data.extend(gameplay[0x908E:0x9095])
    if include_field_ai:
        # Selector wraps the type-minus-35 index to a byte before multiplying by 12.
        field_rows=[stats[0x9662+12*((kind-35)&255)+4] for kind in range(256)]
        data.extend(field_rows)
        data.extend(stats[0x9662+12*((kind-35)&255)+10] for kind in range(256))
        data.extend(stats[0x8BBE:0x8CBE])
        rows=max(field_rows[1:118])+1
        if rows>64 or 0xB12E+48*rows>0xC000:raise ValueError('Unsupported field AI row bounds')
        data.extend(struct.pack('<H',rows))
        data.extend(stats[0xB12E:0xB12E+48*rows])
    if include_field_ai_pass:
        if count!=118:raise ValueError('Unsupported field pass-option type count')
        for player_type in range(count):
            group=word(stats,0x8E1B+2*player_type)
            if not groups<=group<=0x934D-4:raise ValueError('Unsupported field pass-option group')
            data.append(int(word(stats,group+2)==0))
    if include_pass_flight:
        pointer=word(gameplay,0x8D9D);index=6
        data.extend((gameplay[pointer+2*index],gameplay[pointer+2*index+1],gameplay[0x88EB+index]))
        data.extend(gameplay[0x8294:0x8298])
    if include_pass_race:
        for profile in range(256):data.extend(stats[0xAFAE+12*profile+8:0xAFAE+12*profile+12])
    if include_keeper_ai:
        data.append(team_id)
        for profile in range(221):data.extend(stats[0x9662+12*profile+4:0x9662+12*profile+7])
        data.extend(stats[0xBA2E:0xBAB2])
        for player_type in range(count):
            group=word(stats,0x8E1B+2*player_type)
            if not groups<=group<=0x934D-2:raise ValueError('Unsupported keeper AI special-option group')
            data.extend(stats[group:group+2])
    if include_kickoff:
        data.extend(gameplay[0xDC87:0xDCDF]);data.extend(gameplay[0xDC82:0xDC87])
    if include_cell_ai:
        if count!=118:raise ValueError('Unsupported field action2-option type count')
        for player_type in range(count):
            group=word(stats,0x8E1B+2*player_type)
            if not groups<=group<=0x934D-6:raise ValueError('Unsupported field action2-option group')
            data.append(int(word(stats,group+4)==0))
    if include_cell_shots:
        for player_type in range(count):
            group=word(stats,0x8E1B+2*player_type)
            if not groups<=group<=0x934D-2:raise ValueError('Unsupported field shot-option group')
            data.append(int(no_variant_zero_shot_option(stats,word(stats,group))))
    if include_keeper_menu:
        data.extend(menu[0xD366:0xD36A]);data.extend(menu[0xD548:0xD54A])
    if include_pass_defense:
        offset=stats[0x86B5]
        if offset!=0:raise ValueError('Unsupported original pass-defense situation')
        data.extend(stats[0x8BBE:0x8CBE])
        profiles=[stats[0x9662+12*((kind-35)&255)+7+offset] for kind in range(1,118)]
        vector(profiles)
        rows=max(profiles)+1
        if rows>64 or 0xB8AE+48*rows>0xC000:raise ValueError('Unsupported pass-defense AI row bounds')
        data.extend(struct.pack('<H',rows));data.extend(stats[0xB8AE:0xB8AE+48*rows])
        data.extend(struct.pack('<H',count))
        for kind in range(count):
            group=word(stats,0x8E1B+2*kind);pointer=word(stats,group+10)
            data.append(int(bool(pointer)))
            if pointer:
                if not 0x934D<=pointer<0x9460:raise ValueError('Unsupported pass-defense special-option pointer')
                value=stats[pointer];conditional=not bool(value&128)
                if conditional and pointer+1>=0x9460:raise ValueError('Truncated pass-defense teammate condition')
                data.extend((value&127,int(conditional),stats[pointer+1] if conditional else 0))
        # Bank26:$80C2 selects direct table entry (ordinary pass, action1 defense).
        direct=word(gameplay,0x8D93)
        data.extend((gameplay[direct+10],gameplay[direct+11],int(bool(gameplay[0x827C+5]&128))))
        data.extend(gameplay[0x8290:0x8294])
        data.extend(struct.pack('<H',count))
        for kind in range(count):
            group=word(stats,0x8E1B+2*kind);pointer=word(stats,group+8)
            data.append(int(bool(pointer)))
            if pointer:
                if not 0x934D<=pointer<0x9460:raise ValueError('Unsupported pass-defense block-option pointer')
                value=stats[pointer];conditional=not bool(value&128)
                if conditional and pointer+1>=0x9460:raise ValueError('Truncated pass-defense block teammate condition')
                data.extend((value&127,int(conditional),stats[pointer+1] if conditional else 0))
        data.extend((gameplay[direct+8],gameplay[direct+9],int(bool(gameplay[0x827C+4]&128))))
        data.extend(gameplay[0x8290:0x8294])
    if include_first_match:
        opening=ReferenceCartridge(prg);opening.registers[6:8]=[0,2]
        data.extend(opening[0xAA47:0xAA52])
    if include_generic_pass_defense:
        extra=bytes(stats[0x9662+12*((kind-35)&255)+7] for kind in range(118,121))
        if any(row>=rows for row in extra):raise ValueError('Generic pass-defense row outside existing table')
        data.extend(extra)
    if include_pass_action2:
        data.extend((gameplay[direct+12],gameplay[direct+13],int(bool(gameplay[0x827C+6]&128))))
        data.extend(gameplay[0x8290:0x8294])
    return bytes(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--team-id', type=int, help='Explicit observed opposing team ID (3..6)')
    parser.add_argument('--include-field-ai', action='store_true', help='Include provisional-runtime field AI selector tables (CMJSHOTL)')
    parser.add_argument('--include-field-ai-pass', action='store_true', help='Include field AI and zero-pass-option gates (CMJSHOTM)')
    parser.add_argument('--include-pass-flight', action='store_true', help='Include opponent ordinary pass flight comparison (CMJSHOTN)')
    parser.add_argument('--include-pass-race', action='store_true', help='Include free-pass race speeds (CMJSHOTO)')
    parser.add_argument('--include-keeper-ai', action='store_true', help='Include team-specific opposing goalkeeper AI (CMJSHOTP)')
    parser.add_argument('--include-kickoff', action='store_true', help='Include kickoff formations and receiver table (CMJSHOTQ)')
    parser.add_argument('--include-cell-ai', action='store_true', help='Include field action2 option gates (CMJSHOTR)')
    parser.add_argument('--include-cell-shots', action='store_true', help='Include safe variant-zero opponent shot gates (CMJSHOTS)')
    parser.add_argument('--include-keeper-menu', action='store_true', help='Include home goalkeeper menu tables (CMJSHOTT)')
    parser.add_argument('--include-pass-defense', action='store_true', help='Include opponent pass-defense candidate, special-option and comparison tables (CMJSHOTX)')
    parser.add_argument('--include-first-match', action='store_true', help='Include the traced first-match home lineup (CMJSHOTY)')
    parser.add_argument('--include-generic-pass-defense', action='store_true', help='Include first-fixture generic pass-defense profiles (CMJSHOTZ)')
    parser.add_argument('--include-pass-action2', action='store_true', help='Include ordinary pass action2 comparison (CMJSHOTa)')
    args = parser.parse_args()
    if args.output.suffix != '.cmjpack':
        parser.error('Output must use the .cmjpack extension (ignored by Git)')
    try:
        data = export_data(args.rom.read_bytes(), args.team_id, args.include_field_ai, args.include_field_ai_pass, args.include_pass_flight, args.include_pass_race, args.include_keeper_ai, args.include_kickoff, args.include_cell_ai, args.include_cell_shots, args.include_keeper_menu, args.include_pass_defense, args.include_first_match, args.include_generic_pass_defense, args.include_pass_action2)
        with args.output.open('xb') as output:
            output.write(data)
    except (OSError, ValueError) as error:
        parser.exit(1, f'Export stopped: {error}\n')
    print(f'Exported {len(data)} bytes to {args.output}; private data-only development pack.')
    print('Team selection must be supplied by the caller; graphics, audio and text are not included.')


if __name__ == '__main__':
    main()
