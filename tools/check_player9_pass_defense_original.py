#!/usr/bin/env python3
"""Compare a controlled player9 five-defender pass against original AI."""
import argparse
import hashlib
from pathlib import Path
from check_generic_pass_defense_original import original
from export_phase_session import export_session
from trace_ball_recovery_phase import trace_recovery
from trace_action_menu import trace_menu
from verify_ordinary_pass_initialization import original_target
from check_ai_field_candidate_original import original_candidate
from check_ai_field_plan_original import original_choice
from check_field_defense_provenance_original import confirmed_skip
from check_ai_field_preparation_original import original_preparation
from check_human_keeper_menu_original import menu_trace
from check_keeper_saves_original import observe_keeper_save
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
def compare_player21_sequence(raw,sequence,prefix):
 for line in (a.trace/f'native-{sequence}.txt').read_text().splitlines():
  i,phase,first,second,next_phase=map(int,line.split())
  injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
  params={9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase]
  observed,raw=trace_recovery(rom[16:262160],injected,**params)
  expected='player_control_ready' if phase==13 and next_phase==9 else {9:'control_poll_wait',10:'control_clock_pending',12:'ai_cell_decision',13:'field_round_pending'}[next_phase]
  if observed['boundary']!=expected:raise ValueError('Unexpected player21 original boundary')
  compare_represented((a.trace/f'native-{prefix}-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'player21 continuation',str(i))
 return raw

def compare_player21_choice(raw,name):
 info=(a.trace/f'native-{name}.txt').read_text().split();seed=int(info[1]);injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255))
 if original_candidate(rom[16:262160],injected)[2]!=3:raise ValueError('Unexpected player21 candidate')
 result=original_choice(rom[16:262160],injected)
 if (result[0x43b],result[0x43c])!=(2,0):raise ValueError('Unexpected player21 choice')
 compare_represented((a.trace/f'native-{name}.cmjsession').read_bytes(),export_session(result,0),False,'player21 continuation',name)
 return result
raw=compare_player21_sequence(ready,'player21-sequence','player21-continuation')
raw=compare_player21_choice(raw,'player21-choice')
raw=compare_player21_sequence(raw,'player21-after-choice-sequence','player21-after-choice')
raw=compare_player21_choice(raw,'player21-next-choice')
for line in (a.trace/'native-player21-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported=map(int,line.split())
 if unsupported!=-1:raise ValueError('Native player21 continuation stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 if phase==12:
  if original_candidate(rom[16:262160],injected)[2]!=2:raise ValueError('Unexpected player21 pass candidate')
  raw=original_choice(rom[16:262160],injected)
  if (raw[0x43b],raw[0x43c],raw[0x5fc],raw[0x638])!=(1,0,19,79):raise ValueError('Unexpected player21 pass choice')
 else:
  observed,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-player21-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'player21 journey',str(i))
for index,name in ((0,'prepared'),(1,'preclock'),(3,'handoff'),(4,'target'),(5,'flight'),(11,'control'),(12,'ready')):
 original=(a.player3_shot/f'player21-pass-{name}.bin').read_bytes()
 compare_represented((a.trace/f'native-player21-pass-{index}.cmjsession').read_bytes(),export_session(original,0),False,'player21 pass',name)
 if name=='ready' and (original[0x441],original[0x5fb])!=(19,11):raise ValueError('Unexpected player21 pass owner')
raw=(a.player3_shot/'player21-pass-ready.bin').read_bytes()
for line in (a.trace/'native-player19-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported=map(int,line.split())
 if unsupported!=-1:raise ValueError('Native player19 continuation stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 if phase==12:raw=original_choice(rom[16:262160],injected)
 else:observed,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-player19-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'player19 journey',str(i))
if (raw[0x600],raw[0x601],raw[0x602])!=(2,3,5):raise ValueError('Unexpected original player19 defenders')
base=bytearray(raw);base[0x60b:0x60d]=bytes((6,6))
for index,cursor in ((2,0),(5,1)):
 line=(a.trace/'native-player19-contact-sequence.txt').read_text().splitlines()[index].split();seed=int(line[-1]);base[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255));base[0x61e]=cursor
 base=bytearray(confirmed_skip(rom[16:262160],base)[0])
 compare_represented((a.trace/f'native-player19-contact-{index}.cmjsession').read_bytes(),export_session(base,0),False,'player19 skip',str(cursor))
line=(a.trace/'native-player19-contact-sequence.txt').read_text().splitlines()[8].split();seed=int(line[-1]);base[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255));base[0x61e]=2
if original_candidate(rom[16:262160],base)[2]!=3 or original_choice(rom[16:262160],base)[0x43b]!=2:raise ValueError('Unexpected player19 fallback')
for index,name,kwargs in ((9,'prepared',{}),(11,'clock',{'through_clock':True}),(12,'handoff',{'through_return':True}),(13,'ready',{'through_return':True,'through_control':True})):
 original=original_preparation(rom[16:262160],base,**kwargs)
 compare_represented((a.trace/f'native-player19-contact-{index}.cmjsession').read_bytes(),export_session(original,0),False,'player19 fallback',name)
 if name=='ready' and (original[0x441],original[0x5fb],original[0x5f7])!=(19,11,79):raise ValueError('Unexpected player19 possession')
raw=original
for line in (a.trace/'native-player19-after-contact-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Player19 action3 continuation stopped early')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 if phase==12:
  if original_candidate(rom[16:262160],injected)[2]!=4:raise ValueError('Unexpected original player19 action3 candidate')
  raw=original_choice(rom[16:262160],injected)
  if (raw[0x43b],raw[0x43c],raw[0x5fc],raw[0x638])!=(3,0,14,78):raise ValueError('Unexpected player19 action3 target')
 else:observed,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-player19-after-contact-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'player19 action3 journey',str(i))
for native,name in (('prepared','prepared'),('clock','preclock'),('moved','clock'),('flight','flight'),('flight2','flight2'),('contact','control'),('home-control','ready')):
 original=(a.player3_shot/f'player19-action3-{name}.bin').read_bytes()
 compare_represented((a.trace/f'native-player19-action3-{native}.cmjsession').read_bytes(),export_session(original,0),False,'player19 action3',name)
arrival=(a.player3_shot/'player19-action3-arrival.bin').read_bytes()
if (arrival[0x635],arrival[0x637],arrival[0x634],arrival[0x636],arrival[0x5fe],arrival[0x600])!=(84,132,235,37,54,0):raise ValueError('Unexpected original action3 first-leg arrival')
arrival2=(a.player3_shot/'player19-action3-arrival2.bin').read_bytes();home=(a.player3_shot/'player19-action3-ready.bin').read_bytes()
if (arrival2[0x635],arrival2[0x637],arrival2[0x634],arrival2[0x636],arrival2[0x5fe],list(arrival2[0x601:0x601+arrival2[0x600]]))!=(52,132,235,37,6,[3,4,5,7]):raise ValueError('Unexpected original action3 second leg')
if (home[0x441],home[0x5fb],home[0x635],home[0x637],home[0x5f7])!=(3,0,77,135,74):raise ValueError('Unexpected original action3 interception owner')

raw=(a.player3_shot/'player19-action3-ready.bin').read_bytes()
for line in (a.trace/'native-home3-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported=map(int,line.split())
 if unsupported!=-1:raise ValueError('Home player3 continuation stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 observed,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(1,0)},10:{'control_tick':True},13:{'world_inputs':[(1,0)]*22}}[phase])
 compare_represented((a.trace/f'native-home3-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'home player3 journey',str(i))
if (raw[0x600],raw[0x601],raw[0x602])!=(2,15,16):raise ValueError('Unexpected home player3 defenders')
for line in (a.trace/'native-home3-menu-sequence.txt').read_text().splitlines()[:3]:
 i,pressed,unsupported,phase,seed=map(int,line.split());injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255))
 observed,raw=trace_menu(rom[16:262160],injected,mode='open' if i==0 else 'poll',pressed=pressed)
 compare_represented((a.trace/f'native-home3-menu-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'home player3 menu',str(i))
 if i==0:opened=raw
carrier=0x300+12*3;x=opened[carrier+5]|opened[carrier+6]<<8;y=opened[carrier+7]|opened[carrier+8]<<8
for flags in (0,1,3,128,255):
 injected=bytearray(opened);injected[0x443]=flags
 if original_target(rom[16:262160],injected,x,y,return_ram=True)[0x443]!=0:raise ValueError('Original pass target retained move flags')
original=(a.player3_shot/'home3-target.bin').read_bytes()
compare_represented((a.trace/'native-home3-menu-3.cmjsession').read_bytes(),export_session(original,0),True,'home player3 pass','target initialization')
for index,pc in ((1,'d84b-1'),(3,'8637-1'),(4,'8637-2'),(5,'804f-1'),(6,'80b6-1'),(7,'80d9-1'),(8,'80dc-1'),(9,'80b6-2'),(10,'80d9-2'),(11,'80dc-2'),(12,'80ed-1'),(13,'80f0-1'),(14,'de64-1'),(15,'e773-1'),(16,'e7cc-1'),(19,'e0df-1'),(20,'e145-1')):
 original=(a.player3_shot/f'home3-pass-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-home3-target-{index}.cmjsession').read_bytes(),export_session(original,0),index!=20,'home player3 pass',pc)
 if pc.startswith('80d9') and original[0x612]!=0:raise ValueError('Unexpected original home pass defender result')
 if pc=='e145-1' and (original[0x441],original[0x5fb],original[0x635],original[0x637],original[0x5f7])!=(4,0,84,140,68):raise ValueError('Unexpected original home pass recipient')
raw=(a.player3_shot/'home3-pass-e145-1.bin').read_bytes()
for line in (a.trace/'native-home4-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Home player4 continuation stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 observed,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(1,0)},10:{'control_tick':True},13:{'world_inputs':[(1,0)]*22}}[phase])
 compare_represented((a.trace/f'native-home4-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'home player4 journey',str(i))
if (raw[0x441],raw[0x5f7],raw[0x616],raw[0x600],list(raw[0x601:0x606]))!=(4,63,2,5,[12,14,17,18,19]):raise ValueError('Unexpected player4 five-defender contact')
seed=int((a.trace/'native-home4-menu-seed.txt').read_text());injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255))
observed,raw=trace_menu(rom[16:262160],injected)
compare_represented((a.trace/'native-home4-menu.cmjsession').read_bytes(),export_session(raw,0),False,'home player4','menu')
for index,pc in ((5,'d84b-1'),(7,'8637-1'),(8,'8637-2'),(9,'8637-3'),(10,'8637-4'),(11,'8637-5'),(12,'804f-1'),(13,'80b6-1'),(14,'80d9-1'),(15,'80dc-1'),(16,'80b6-2'),(17,'80d9-2'),(18,'e0df-1')):
 raw=(a.player3_shot/f'home4-pass-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-home4-action-{index}.cmjsession').read_bytes(),export_session(raw,0),index!=18,'home player4 pass',pc)
 if pc=='8637-5' and list(raw[0x60b:0x610])!=[1,2,1,2,0]:raise ValueError('Unexpected home player4 defender choices')
 if pc=='80d9-1' and raw[0x612]!=0:raise ValueError('Unexpected first defender result')
 if pc=='80d9-2' and (raw[0x442],raw[0x43d],raw[0x43e],raw[0x612])!=(14,2,128,3):raise ValueError('Unexpected player14 interception')
raw=(a.player3_shot/'home4-pass-e145-1.bin').read_bytes()
compare_represented((a.trace/'native-home4-intercepted-control.cmjsession').read_bytes(),export_session(raw,0),False,'home player4 pass','interception control')
if (raw[0x441],raw[0x5fb],raw[0x635],raw[0x637],raw[0x5f7])!=(14,11,98,139,60):raise ValueError('Unexpected interception possession')
raw=(a.player3_shot/'home4-pass-e145-1.bin').read_bytes()
for line in (a.trace/'native-player14-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Player14 continuation stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 if phase==12:raw=original_choice(rom[16:262160],injected)
 else:observed,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-player14-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'player14 journey',str(i))
if (raw[0x441],raw[0x5f7],raw[0x616],raw[0x600],raw[0x601])!=(14,55,1,1,3):raise ValueError('Unexpected player14 contact')
lines=(a.trace/'native-player14-contact-sequence.txt').read_text().splitlines();seed=int(lines[2].split()[-1]);base=bytearray(raw);base[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255));base[0x61e]=0;base[0x60b]=6;base=bytearray(confirmed_skip(rom[16:262160],base)[0])
compare_represented((a.trace/'native-player14-contact-2.cmjsession').read_bytes(),export_session(base,0),False,'player14 contact','defender skip')
seed=int(lines[5].split()[-1]);base[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255));base[0x61e]=1;choice=original_choice(rom[16:262160],base)
if (choice[0x43b],choice[0x43c],choice[0x5fc])!=(1,0,21):raise ValueError('Unexpected player14 pass choice')
for index,name,kwargs in ((6,'prepared',{}),(8,'clock',{'through_clock':True}),(19,'handoff',{'through_return':True})):
 original=original_preparation(rom[16:262160],base,**kwargs)
 compare_represented((a.trace/f'native-player14-contact-{index}.cmjsession').read_bytes(),export_session(original,0),False,'player14 pass',name)
original=original_preparation(rom[16:262160],base,through_return=True,through_control=True)
compare_represented((a.trace/'native-player14-recipient-control.cmjsession').read_bytes(),export_session(original,0),False,'player14 pass','recipient control')
if (original[0x441],original[0x5fb],original[0x635],original[0x637],original[0x5f7])!=(21,11,69,167,54):raise ValueError('Unexpected player14 pass recipient')
raw=original
for line in (a.trace/'native-player21-return-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Player21 return continuation stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 if phase==12:raw=original_choice(rom[16:262160],injected)
 else:observed,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-player21-return-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'player21 return',str(i))
if (raw[0x441],raw[0x5f7],raw[0x616],raw[0x600],raw[0x601])!=(21,49,1,1,2):raise ValueError('Unexpected player21 contact')
lines=(a.trace/'native-player21-return-contact.txt').read_text().splitlines();seed=int(lines[2].split()[-1]);base=bytearray(raw);base[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255));base[0x61e]=0;base[0x60b]=6;base=bytearray(confirmed_skip(rom[16:262160],base)[0])
seed=int(lines[5].split()[-1]);base[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255));base[0x61e]=1
if (original_candidate(rom[16:262160],base)[2],original_choice(rom[16:262160],base)[0x43b])!=(1,0):raise ValueError('Unexpected player21 shot choice')
for index,pc in ((6,'804f-1'),(8,'80ed-1'),(9,'c618-1'),(10,'dd08-1'),(11,'e773-1'),(13,'e773-2'),(15,'e773-3'),(17,'e773-4'),(19,'e773-5'),(21,'e773-6'),(22,'e7cc-1'),(23,'dd1c-1')):
 original=(a.player3_shot/f'player21-shot-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-player21-return-contact-{index}.cmjsession').read_bytes(),export_session(original,0),False,'player21 shot',pc)
 if pc=='e7cc-1' and (original[0x600],original[0x601])!=(1,5):raise ValueError('Unexpected shot flight defender')
entry=(a.player3_shot/'player21-shot-dd1c-1.bin').read_bytes();stages=menu_trace(rom[16:262160],entry,1,seed)
for index,name in ((0,'open'),(1,'selected'),(2,'compared')):
 original=bytearray(stages[name]);original[0x600]=0;expected=bytearray(export_session(original,0));expected[28]=1;expected[309]=0
 compare_represented((a.trace/f'native-player21-shot-keeper-{index}.cmjsession').read_bytes(),expected,False,'player21 shot goalkeeper',name)
if (stages['compared'][0x43d],stages['compared'][0x612])!=(0,2):raise ValueError('Unexpected goalkeeper save result')
saved,phase,_=observe_keeper_save(rom[16:262160],stages['compared'])
compare_represented((a.trace/'native-player21-shot-save.cmjsession').read_bytes(),export_session(saved,0),False,'player21 shot','save')
observed,out=trace_recovery(rom[16:262160],saved)
if observed['boundary']!='boundary_restart' or (out[0x635],out[0x637],out[0x634],out[0x636])!=(44,116,7,91):raise ValueError('Unexpected original keeper rebound boundary')
compare_represented((a.trace/'native-player21-shot-recovery.cmjsession').read_bytes(),export_session(out,0),False,'player21 shot','boundary restart')
for native,pc in (('setup','92f8'),('position','9306'),('taker','9318'),('formation','9328'),('clock','9342')):
 original=(a.player3_shot/f'player21-out-{pc}-1.bin').read_bytes()
 compare_represented((a.trace/f'native-player21-corner-{native}.cmjsession').read_bytes(),export_session(original,0),True,'player21 corner',native)
for native,pc in (('flight','e773-1'),('contact','8bdf-2'),('out','8ca4-2')):
 original=(a.player3_shot/f'player21-out-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-player21-corner-{native}.cmjsession').read_bytes(),export_session(original,0),False,'first corner pass',native)
contact=(a.player3_shot/'player21-out-8870-1.bin').read_bytes()
if (contact[0x442],contact[0x43d],contact[0x612])!=(2,2,2):raise ValueError('Unexpected first corner defender result')
for native,pc in (('setup','92f8-2'),('position','9306-2'),('taker','9318-2'),('formation','9328-2'),('clock','9342-2')):
 original=(a.player3_shot/f'player21-out-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-player21-corner2-{native}.cmjsession').read_bytes(),export_session(original,0),True,'second corner',native)
for native,pc in (('flight','e773-10'),('received','e0df-1'),('control','e145-1')):
 original=(a.player3_shot/f'player21-out-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-player21-corner2-{native}.cmjsession').read_bytes(),export_session(original,0),False,'second corner pass',native)
if (original[0x441],original[0x5fb],original[0x635],original[0x637],original[0x5f7])!=(17,11,60,92,8):raise ValueError('Unexpected second corner recipient')
raw=(a.player3_shot/'player21-out-e145-1.bin').read_bytes()
for line in (a.trace/'native-corner-player17-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Corner player17 continuation stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 if phase==12:
  raw=(a.player3_shot/'corner17-pass-e1b2-1.bin').read_bytes()
  if (raw[0xe2],raw[0xe3],raw[0x43b],raw[0x5fc],raw[0x638])!=(first,second,1,19,5):raise ValueError('Unexpected original player17 pass choice')
 else:observed,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-corner-player17-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'corner player17',str(i))
for index,pc in ((0,'e1cb-1'),(1,'80ed-1'),(3,'c612-1'),(4,'de64-1'),(5,'e773-1'),(13,'8b4a-1')):
 original=(a.player3_shot/f'corner17-pass-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-corner17-pass-{index}.cmjsession').read_bytes(),export_session(original,0),False,'corner player17 pass',pc)
 if pc=='e773-1' and (original[0x61a],original[0x628])!=(38,128):raise ValueError('Unexpected original pass flight threshold')
original=(a.player3_shot/'corner17-pass-8b73-1.bin').read_bytes()
compare_represented((a.trace/'native-corner19-area-prepared.cmjsession').read_bytes(),export_session(original,0),False,'corner player19','area setup')
if (original[0x441],original[0x600],original[0x601])!=(19,1,4):raise ValueError('Unexpected goal-area selection')
original=(a.player3_shot/'corner17-pass-d3a6-1.bin').read_bytes()
assert original[0x600:0x605]==bytes((4,0,3,4,7))
exportable=bytearray(original);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-corner19-defense-initial.cmjsession').read_bytes(),expected,False,'corner player19','defense initialization')
print('Matched player9 menu, five defender choices and contacts, flight and player6 recovery:',choices)
