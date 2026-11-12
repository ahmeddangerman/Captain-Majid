#!/usr/bin/env python3
"""Compare a controlled player9 five-defender pass against original AI."""
import argparse
import hashlib
from pathlib import Path
from check_generic_pass_defense_original import original
from export_phase_session import export_session
from trace_ball_recovery_phase import trace_recovery
from check_ai_field_candidate_original import original_candidate
from check_ai_field_plan_original import original_choice
from verify_action_strength import PAYLOAD_SHA256
from verify_player7_pass_end_to_end import compare_represented

p=argparse.ArgumentParser(description=__doc__)
for name in ('rom','injected','trace','menu','contacts','prepared','player6-area','player6-shot','player4-shot','player3-shot'):p.add_argument('--'+name,type=Path,required=True)
a=p.parse_args();rom=a.rom.read_bytes();base=bytearray(a.injected.read_bytes())
if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256 or len(base)!=2048:raise ValueError('Expected verified original ROM and private RAM')
menu=(a.trace/'native-player9-menu.cmjsession').read_bytes();compare_represented(menu,export_session(a.menu.read_bytes(),0),False,'player9','five-defender menu')
choices=[]
for i,actor in enumerate((13,14,18,19,20)):
 s=(a.trace/f'native-player9-defense-{i}.cmjsession').read_bytes();raw=bytearray(base);raw[0xe2],raw[0xe3]=s[26:28];raw[0x442]=actor
 action,subtype=original(rom[16:262160],raw,actor);choices.append(action)
 if subtype or s[-11]!=i+1 or list(s[-10:-5])[:i+1]!=choices or any(s[-5:]):raise ValueError('Original defender choices disagree')
 compare_represented(s,export_session(raw,0),True,'player9',f'defender {i+1}')
compare_represented((a.trace/'native-player9-pass-prepared.cmjsession').read_bytes(),export_session(a.prepared.read_bytes(),0),True,'player9','pass preparation')
for i,label in enumerate(('first','second','third','contact-4','contact-5'),1):
 for kind,pc in (('prepared','80b6'),('compared','80d9'),('clocked','80dc')):
  s=(a.trace/f'native-player9-{label}-{kind}.cmjsession').read_bytes();raw=(a.contacts/f'{pc}-{i}.bin').read_bytes()
  compare_represented(s,export_session(raw,0),True,'player9',f'contact {i} {kind}')
  if kind!='prepared' and raw[0x612]!=0:raise ValueError('Unexpected original defender result')
for name,pc in (('dispatch','80ed-1'),('pass-moved','80f0-1'),('pass-target','de64-1'),('flight','e773-1'),('flight-step-1','e773-2'),('flight-step-3','e773-3'),('flight-step-5','e773-4'),('flight-step-7','e773-5'),('flight-step-8','8845-1'),('arrival','888a-1'),('race','8c31-1')):
 s=(a.trace/f'native-player9-{name}.cmjsession').read_bytes();raw=(a.contacts/f'{pc}.bin').read_bytes()
 compare_represented(s,export_session(raw,0),True,'player9',name)
 if name in ('dispatch','pass-moved') and (s[413],raw[0x616])!=(5,5):raise ValueError('Original exhausted defender cursor differs')
 if name=='race' and (raw[0x441],raw[0x5fb])!=(6,0):raise ValueError('Unexpected original pass recovery owner')
for name,pc in (('area','8b4a-1'),('area-prepared','8b73-1'),('menu','d5e5-1')):
 raw=(a.player6_area/f'{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-player6-{name}.cmjsession').read_bytes(),export_session(raw,0),False,'player6',name)
 if raw[0x441]!=6 or raw[0x600]!=0:raise ValueError('Unexpected player6 goal-area state')
for name,pc,extended in (('confirmed','8b8f-1',False),('prepared','82a0-1',True),('clocked','83f5-1',True),('moved','83f8-1',True),('dispatch','c618-1',True)):
 raw=(a.player6_shot/f'{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-player6-shot-{name}.cmjsession').read_bytes(),export_session(raw,0),extended,'player6 shot',name)
 if raw[0x441]!=6 or raw[0x43b]!=0 or raw[0x43c]!=2:raise ValueError('Unexpected player6 shot state')
for name,pc in (('shot-target','dd08-1'),('shot-flight','e773-6'),('shot-flight-1','e773-7'),('shot-flight-3','e773-8'),('shot-flight-5','e773-9'),('shot-flight-7','e773-10'),('shot-flight-9','e773-11'),('shot-flight-11','e773-12'),('shot-flight-12','e7cc-2'),('keeper-entry','dd1c-1'),('keeper-prompt','8509-1'),('keeper-result','8599-1'),('save','8bdf-1'),('recovered','8c31-2')):
 raw=bytearray((a.player6_shot/f'{pc}.bin').read_bytes());count=raw[0x600];raw[0x600]=0
 expected=bytearray(export_session(raw,0));expected[28]=count
 if count: expected[309]=11
 compare_represented((a.trace/f'native-player6-{name}.cmjsession').read_bytes(),expected,False,'player6 shot',name)
 if name=='keeper-result' and (raw[0x43d],raw[0x612])!=(1,2):raise ValueError('Unexpected original goalkeeper result')
 if name=='recovered' and (raw[0x441],raw[0x5fb],raw[0x634],raw[0x636])!=(4,0,79,152):raise ValueError('Unexpected original rebound owner or fraction')
for name,pc in (('area','8b4a-2'),('area-prepared','8b73-2'),('menu','d5e5-3')):
 raw=bytearray((a.player6_shot/f'{pc}.bin').read_bytes());count=raw[0x600];raw[0x600]=0
 expected=bytearray(export_session(raw,0));expected[28]=count
 if count: expected[309]=11
 compare_represented((a.trace/f'native-player4-{name}.cmjsession').read_bytes(),expected,False,'player4 rebound',name)
 if raw[0x441]!=4:raise ValueError('Unexpected player4 goal-area owner')
for name,pc,extended in (('shot-confirmed','8b8f-2',False),('keeper-chosen','8637-2',True),('shot-prepared','82a0-2',True),('keeper-compared','8385-1',True),('keeper-clocked','83f5-2',True),('keeper-moved','83f8-2',True),('recovery-entry','8bdf-2',True),('recovered','8c31-3',True)):
 raw=bytearray((a.player4_shot/f'{pc}.bin').read_bytes());count=raw[0x600];raw[0x600]=0
 expected=bytearray(export_session(raw,0));expected[28]=count
 if count: expected[309]=11
 compare_represented((a.trace/f'native-player4-{name}.cmjsession').read_bytes(),expected,extended,'player4 shot',name)
 if name=='keeper-chosen' and (raw[0x60b],raw[0xe2],raw[0xe3])!=(3,164,142):raise ValueError('Unexpected player4 goalkeeper choice')
 if name=='recovered' and (raw[0x441],raw[0x5fb],raw[0x634],raw[0x636])!=(3,0,79,152):raise ValueError('Unexpected player4 rebound owner or fractions')
for name,pc in (('area','8b4a-3'),('area-prepared','8b73-3'),('menu','d5e5-5')):
 raw=bytearray((a.player4_shot/f'{pc}.bin').read_bytes());keeper=(raw[0x600]==1 and raw[0x601]==11)
 if keeper:raw[0x600]=0
 expected=bytearray(export_session(raw,0))
 if keeper:expected[28]=1;expected[309]=11
 compare_represented((a.trace/f'native-player3-{name}.cmjsession').read_bytes(),expected,False,'player3 rebound',name)
 if raw[0x441]!=3:raise ValueError('Unexpected player3 goal-area owner')
for name,pc,extended in (('shot-confirmed','8b8f-3',False),('defender-1','8637-3',True),('defender-2','8637-4',True),('defender-3','8637-5',True),('defender-4','8637-6',True),('shot-prepared','82a0-3',True)):
 raw=(a.player3_shot/f'{pc}.bin').read_bytes();state=(a.trace/f'native-player3-{name}.cmjsession').read_bytes()
 compare_represented(state,export_session(raw,0),extended,'player3 shot',name)
 if name.startswith('defender-'):
  count=int(name[-1]);expected=list(raw[0x60b:0x60b+count])+[255]*(4-count)
  if state[373]!=1 or list(state[374:378])!=expected or any(raw[0x606:0x606+count]):raise ValueError('Player3 defender choices disagree')
for i in range(1,5):
 for kind,pc,count in (('prepared','8349',i+1),('compared','8385',i+1),('clocked','83c3',i)):
  raw=(a.player3_shot/f'{pc}-{count}.bin').read_bytes();state=(a.trace/f'native-player3-contact-{i}-{kind}.cmjsession').read_bytes()
  compare_represented(state,export_session(raw,0),True,'player3 area',f'contact {i} {kind}')
  if raw[0x442]!=(12,15,17,21)[i-1] or (kind!='prepared' and raw[0x612]!=0):raise ValueError('Unexpected player3 area contact')
raw=(a.player3_shot/'83f5-3.bin').read_bytes();state=(a.trace/'native-player3-prefix-clocked.cmjsession').read_bytes()
compare_represented(state,export_session(raw,0),True,'player3 area','prefix clock')
if (raw[0x616],raw[0x5f7],raw[0x612])!=(4,92,0):raise ValueError('Unexpected player3 area prefix')
for name,pc in (('shot-moved','83f8-3'),('shot-dispatch','c618-2'),('shot-target','dd08-2'),('keeper-entry','dd1c-2'),('keeper-ready','8509-2'),('keeper-decided','8599-2'),('goal','85f0-1'),('restart','85dd-1')):
 raw=bytearray((a.player3_shot/f'{pc}.bin').read_bytes());keeper=(raw[0x600]==1 and raw[0x601]==11)
 if keeper:raw[0x600]=0
 expected=bytearray(export_session(raw,0))
 if keeper:expected[28]=1;expected[309]=11
 compare_represented((a.trace/f'native-player3-{name}.cmjsession').read_bytes(),expected,name!='restart','player3 same-cell shot',name)
 if name=='goal' and (raw[0x28],raw[0x29]) != (1,0):
  raise ValueError('Unexpected original score')
for name,pc in (('formation','daf5-1'),('receiver','db0b-1'),('ball','db0e-1'),('handoff','db1e-1'),('control','control-ready')):
 raw=bytearray((a.player3_shot/f'{pc}.bin').read_bytes());keeper=(raw[0x600]==1 and raw[0x601]==11)
 if keeper:raw[0x600]=0
 expected=bytearray(export_session(raw,0))
 if keeper:expected[28]=1;expected[309]=11
 compare_represented((a.trace/f'native-player3-kickoff-{name}.cmjsession').read_bytes(),expected,False,'player3 opponent kickoff',name)
raw=(a.player3_shot/'control-ready.bin').read_bytes()
for i in range(11):
 state=(a.trace/f'native-player3-kickoff-poll-{i}.cmjsession').read_bytes()
 injected=bytearray(raw);injected[0xe2:0xe4]=state[26:28]
 observed,raw=trace_recovery(rom[16:262160],injected,control_input=(0,0))
 compare_represented(state,export_session(raw,0),False,'player3 kickoff control',f'poll {i+1}')
 if observed['boundary']!=('control_clock_pending' if i==10 else 'control_poll_wait'):raise ValueError('Unexpected original kickoff control boundary')
clock=(a.trace/'native-player3-kickoff-clock.cmjsession').read_bytes()
injected=bytearray(raw);injected[0xe2:0xe4]=clock[26:28]
observed,raw=trace_recovery(rom[16:262160],injected,control_tick=True)
if observed['boundary']!='field_round_pending':raise ValueError('Unexpected kickoff clock boundary')
compare_represented(clock,export_session(raw,0),False,'player3 kickoff control','clock')
round_state=(a.trace/'native-player3-kickoff-round.cmjsession').read_bytes()
injected=bytearray(raw);injected[0xe2:0xe4]=(a.trace/'native-player3-kickoff-round-random.bin').read_bytes()
observed,raw=trace_recovery(rom[16:262160],injected,world_inputs=[(0,0)]*22)
if observed['boundary']!='player_control_ready':raise ValueError('Unexpected kickoff field-round boundary')
compare_represented(round_state,export_session(raw,0),False,'player3 kickoff control','field round')
for line in (a.trace/'native-player3-kickoff-sequence.txt').read_text().splitlines():
 i,phase,first,second,next_phase=map(int,line.split())
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 params={9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase]
 observed,raw=trace_recovery(rom[16:262160],injected,**params)
 state=(a.trace/f'native-player3-continuation-{i}.cmjsession').read_bytes()
 compare_represented(state,export_session(raw,0),False,'player16 continuation',str(i))
 expected='player_control_ready' if phase==13 and next_phase==9 else {9:'control_poll_wait',10:'control_clock_pending',12:'ai_cell_decision',13:'field_round_pending'}[next_phase]
 if observed['boundary']!=expected:raise ValueError('Unexpected original player16 boundary')
choice_info=(a.trace/'native-player3-opponent-choice.txt').read_text().split();seed=int(choice_info[1])
injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255))
if original_candidate(rom[16:262160],injected)[2]!=2:raise ValueError('Unexpected player16 original candidate')
choice=original_choice(rom[16:262160],injected)
if (choice[0x43b],choice[0x43c],choice[0x5fc],choice[0x638])!=(1,0,21,119):raise ValueError('Unexpected original player16 pass')
compare_represented((a.trace/'native-player3-opponent-choice.cmjsession').read_bytes(),export_session(choice,0),False,'player16 continuation','pass choice')
for index,name in ((0,'prepared'),(1,'preclock'),(3,'handoff'),(4,'target'),(5,'flight'),(17,'post-contact'),(18,'control'),(19,'ready')):
 raw=(a.player3_shot/f'opponent-pass-{name}.bin').read_bytes()
 compare_represented((a.trace/f'native-player3-opponent-pass-{index}.cmjsession').read_bytes(),export_session(raw,0),False,'player16 pass',name)
arrival=(a.player3_shot/'opponent-pass-arrival.bin').read_bytes()
contact=(a.player3_shot/'opponent-pass-contact-compared.bin').read_bytes()
ready=(a.player3_shot/'opponent-pass-ready.bin').read_bytes()
if (arrival[0x600],arrival[0x601],contact[0x612],ready[0x441],ready[0x5fb])!=(1,6,0,21,11):raise ValueError('Unexpected original defender contact or pass owner')
print('Matched player9 menu, five defender choices and contacts, flight and player6 recovery:',choices)
