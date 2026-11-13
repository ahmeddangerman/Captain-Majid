#!/usr/bin/env python3
"""Compare a controlled player9 five-defender pass against original AI."""
import argparse
import hashlib
from pathlib import Path
from check_generic_pass_defense_original import original
from export_phase_session import export_session
from trace_area_defense_poll import trace_defense_poll
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
menu=original
for slot in range(4):
 for label,pressed,preparation in (('direction',1,False),('confirm',128,False),('prepare',128,True)):
  observation,menu=trace_defense_poll(rom[16:262160],menu,pressed,preparation=preparation)
  captured=(a.player3_shot/f'corner19-menu-{slot}-{label}.bin').read_bytes()
  if menu!=captured:raise ValueError('Corner defensive menu trace differs from captured original')
 if menu[0x60b+slot]!=(4 if slot==0 else 3):raise ValueError('Corner defensive direction differs by player role')

menu=original
for line in (a.trace/'native-corner19-menu.txt').read_text().splitlines():
 i,phase,pressed,unsupported,rng=map(int,line.split())
 if unsupported!=-1:raise ValueError('Corner goal-area defensive menu stopped')
 if phase!=67:
  injected=bytearray(menu);injected[0xe2:0xe4]=bytes((rng&255,(rng>>8)&255))
  observation,menu=trace_defense_poll(rom[16:262160],injected,128 if phase==71 else pressed,preparation=phase==71)
 exportable=bytearray(menu);exportable[0x601]=10
 expected=bytearray(export_session(exportable,0));expected[309]=0
 actual=(a.trace/f'native-corner19-menu-{i}.cmjsession').read_bytes()
 compare_represented(actual,expected,False,'corner defensive menu',str(i))
 actions=list(menu[0x60b:0x60f]);subtypes=[menu[0x606+j] if actions[j]!=255 else 0 for j in range(4)]
 expected_choices=bytes((1,*actions,*subtypes,menu[0x620],menu[0x1e] if phase in (69,70) and pressed==128 and i!=13 else 0))
 if actual[372:383]!=expected_choices:raise ValueError(f'Corner defensive menu choices differ at {i}: {list(actual[372:383])} vs {list(expected_choices)}')

original=(a.player3_shot/'corner19-attack-829d-1.bin').read_bytes()
if (original[0x43b],original[0x43c],original[0xe2],original[0xe3])!=(0,1,221,123):raise ValueError('Unexpected corner goal-area attack')
exportable=bytearray(original);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-corner19-attack-choice.cmjsession').read_bytes(),expected,False,'corner player19','attack choice')
original=(a.player3_shot/'corner19-attack-82a0-1.bin').read_bytes()
if original[0x43f]!=90:raise ValueError('Unexpected corner shot cost')
exportable=bytearray(original);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-corner19-attack-prepared.cmjsession').read_bytes(),expected,False,'corner player19','attack preparation')
original=(a.player3_shot/'corner19-attack-814b-1.bin').read_bytes()
if (original[0x442],original[0x612])!=(3,3):raise ValueError('Unexpected corner shot contact')
exportable=bytearray(original);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-corner19-shot-contact.cmjsession').read_bytes(),expected,False,'corner player19','defender contact')
contact=(a.trace/'native-corner19-shot-contact.cmjsession').read_bytes()
if contact[398]!=1 or contact[399]!=1 or contact[400]!=3 or contact[407]!=original[0x71] or contact[408]!=original[0x619] or contact[410]!=3:raise ValueError('Corner shot saved contact details differ')
original=(a.player3_shot/'corner19-attack-83f5-1.bin').read_bytes()
exportable=bytearray(original);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-corner19-contact-clock.cmjsession').read_bytes(),expected,False,'corner player19','contact clock')
original=(a.player3_shot/'corner19-attack-83f8-1.bin').read_bytes()
exportable=bytearray(original);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-corner19-contact-world.cmjsession').read_bytes(),expected,False,'corner player19','field update')
from export_field_state import export_state
original=(a.player3_shot/'corner19-attack-df0d-1.bin').read_bytes()
if (original[0x441],original[0x5fb],original[0x635],original[0x637])!=(13,11,76,84):raise ValueError('Unexpected corner recovery winner')
if (a.trace/'native-corner19-race-field.cmjstate').read_bytes()!=export_state(original):raise ValueError('Corner loose-ball race field differs')
original=(a.player3_shot/'corner19-attack-e145-1.bin').read_bytes()
compare_represented((a.trace/'native-corner19-player13-control.cmjsession').read_bytes(),export_session(original,0),False,'corner player13','control')
raw=original
for line in (a.trace/'native-corner13-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Corner player13 continuation stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 if phase==12:
  raw=(a.player3_shot/'corner13-decision-e1b2-1.bin').read_bytes()
  if (raw[0xe2],raw[0xe3],raw[0x43b],raw[0x5fc],raw[0x638])!=(first,second,1,16,31):raise ValueError('Unexpected corner player13 pass choice')
 else:observed,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-corner13-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'corner player13',str(i))
for index,pc in ((0,'e1cb-1'),(1,'80ed-1'),(3,'c612-1'),(4,'de64-1'),(5,'e773-1'),(19,'8b4a-1')):
 original=(a.player3_shot/f'corner13-decision-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-corner13-pass-{index}.cmjsession').read_bytes(),export_session(original,0),False,'corner player13 pass',pc)
original=(a.player3_shot/'corner13-decision-8b73-1.bin').read_bytes()
compare_represented((a.trace/'native-corner16-area-prepared.cmjsession').read_bytes(),export_session(original,0),False,'corner player16','area setup')
original=(a.player3_shot/'corner16-area-d3a6-1.bin').read_bytes()
if original[0x600:0x602]!=bytes((1,0)):raise ValueError('Unexpected corner player16 defensive lineup')
exportable=bytearray(original);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
actual=(a.trace/'native-corner16-defense-initial.cmjsession').read_bytes()
compare_represented(actual,expected,False,'corner player16','defense initialization')
if actual[372:383]!=bytes((1,255,255,255,255,0,0,0,0,original[0x620],0)):raise ValueError('Corner player16 defensive initialization differs')
menu=original
for line in (a.trace/'native-corner16-menu.txt').read_text().splitlines():
 i,phase,pressed,unsupported,rng=map(int,line.split())
 if unsupported!=-1:raise ValueError('Corner player16 defensive menu stopped')
 if phase!=67:
  injected=bytearray(menu);injected[0xe2:0xe4]=bytes((rng&255,(rng>>8)&255))
  observation,menu=trace_defense_poll(rom[16:262160],injected,128 if phase==71 else pressed,preparation=phase==71)
 exportable=bytearray(menu);exportable[0x601]=10
 expected=bytearray(export_session(exportable,0));expected[309]=0
 actual=(a.trace/f'native-corner16-menu-{i}.cmjsession').read_bytes()
 compare_represented(actual,expected,False,'corner player16 menu',str(i))
 actions=list(menu[0x60b:0x60f]);subtypes=[menu[0x606+j] if actions[j]!=255 else 0 for j in range(4)]
 expected_choices=bytes((1,*actions,*subtypes,menu[0x620],menu[0x1e] if phase in (69,70) and pressed==128 and i!=4 else 0))
 if actual[372:383]!=expected_choices:raise ValueError(f'Corner player16 menu choices differ at {i}')
for name,pc in (('choice','829d-1'),('prepared','82a0-1')):
 original=(a.player3_shot/f'corner16-attack-{pc}.bin').read_bytes()
 exportable=bytearray(original);exportable[0x601]=10
 expected=bytearray(export_session(exportable,0));expected[309]=0
 compare_represented((a.trace/f'native-corner16-attack-{name}.cmjsession').read_bytes(),expected,False,'corner player16 shot',name)
original=(a.player3_shot/'corner16-attack-83f5-1.bin').read_bytes()
exportable=bytearray(original);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-corner16-attack-clock.cmjsession').read_bytes(),expected,False,'corner player16 shot','clock')
original=(a.player3_shot/'corner16-attack-83f8-1.bin').read_bytes()
exportable=bytearray(original);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-corner16-attack-world.cmjsession').read_bytes(),expected,False,'corner player16 shot','field update')
for index,pc in ((0,'e773-1'),(2,'e773-2'),(5,'e7cc-1')):
 original=(a.player3_shot/f'corner16-attack-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-corner16-shot-flight-{index}.cmjsession').read_bytes(),export_session(original,0),False,'corner player16 shot flight',pc)
 if pc=='e7cc-1' and (original[0x600],original[0x601],original[0x635],original[0x637])!=(1,2,52,124):raise ValueError('Unexpected corner shot flight defender')
original=(a.player3_shot/'corner16-attack-84f8-1.bin').read_bytes()
compare_represented((a.trace/'native-corner16-flight-contact.cmjsession').read_bytes(),export_session(original,0),False,'corner player16 shot','defender contact')
original=(a.player3_shot/'corner16-attack-8509-1.bin').read_bytes()
exportable=bytearray(original);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-corner16-keeper-entry.cmjsession').read_bytes(),expected,False,'corner player16 shot','keeper entry')
raw=(a.player3_shot/'corner16-attack-dd1c-1.bin').read_bytes()
checks=menu_trace(rom[16:262160],raw,1,4169960476)
for index,name in ((0,'open'),(1,'selected'),(2,'compared')):
 original=checks[name];exportable=bytearray(original);exportable[0x601]=10
 expected=bytearray(export_session(exportable,0));expected[309]=0
 compare_represented((a.trace/f'native-corner16-keeper-menu-{index}.cmjsession').read_bytes(),expected,False,'corner player16 goalkeeper',name)
if checks['compared'][0x612]!=3:raise ValueError('Unexpected corner goalkeeper result')
original=(a.player3_shot/'corner16-keeper-save.bin').read_bytes()
if (original[0x441],original[0x5fb])!=(0,0):raise ValueError('Unexpected corner goalkeeper possession')
exportable=bytearray(original);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-corner16-keeper-catch.cmjsession').read_bytes(),expected,False,'corner player16 goalkeeper','catch')
for index,name in ((0,'chosen'),(1,'prepared'),(2,'flight_start'),(3,'flight_setup')):
 original=(a.player3_shot/f'corner16-outlet-seeded-{name}.bin').read_bytes()
 exportable=bytearray(original)
 if exportable[0x600]:exportable[0x601]=10
 expected=bytearray(export_session(exportable,0))
 if original[0x600]:expected[309]=0
 compare_represented((a.trace/f'native-corner16-outlet-{index}.cmjsession').read_bytes(),expected,False,'corner goalkeeper outlet',name)
for native,original_name in (('11','8b4a'),('home-area','8b73')):
 original=(a.player3_shot/f'corner16-outlet-{original_name}.bin').read_bytes()
 compare_represented((a.trace/f'native-corner16-outlet-{native}.cmjsession').read_bytes(),export_session(original,0),False,'corner goalkeeper outlet',original_name)
original=(a.player3_shot/'corner5-area-menu.bin').read_bytes()
if (original[0x600],original[0x601])!=(1,18):raise ValueError('Unexpected corner outlet goal-area defender')
compare_represented((a.trace/'native-corner5-area-menu.cmjsession').read_bytes(),export_session(original,0),False,'corner home player5','area menu')
raw=original
for line in (a.trace/'native-corner5-choice.txt').read_text().splitlines():
 i,pressed,unsupported,phase,rng=map(int,line.split())
 if unsupported!=-1:raise ValueError('Corner player5 pass choice stopped')
 if i<2:
  injected=bytearray(raw);injected[0xe2:0xe4]=bytes((rng&255,(rng>>8)&255));_,raw=trace_menu(rom[16:262160],injected,'poll',pressed)
 else:raw=(a.player3_shot/'corner5-pass-d80a-1.bin').read_bytes()
 compare_represented((a.trace/f'native-corner5-choice-{i}.cmjsession').read_bytes(),export_session(raw,0),i==2,'corner home player5',str(i))
for index,pc in ((0,'d949-1'),(1,'d84b-1'),(2,'8b8f-1'),(3,'8637-1'),(4,'8ff2-1')):
 original=(a.player3_shot/f'corner5-target-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-corner5-target-{index}.cmjsession').read_bytes(),export_session(original,0),True,'corner player5 target',pc)
 if original[0x624]!=39:raise ValueError('Unexpected corner player5 target cell')
if original[0x5fc]!=255:raise ValueError('Unexpected corner player5 free pass target')
if original[0x60b]!=4:raise ValueError('Unexpected corner player5 area defender choice')
record=(a.trace/'native-corner5-target-3.cmjsession').read_bytes()
if record[-11]!=1 or record[-10]!=4:raise ValueError('Corner area defense choice differs')
original=(a.player3_shot/'corner5-target-814b-1.bin').read_bytes()
if (original[0x442],original[0x43d],original[0x612])!=(18,4,3):raise ValueError('Unexpected corner player5 area contact')
contact=(a.trace/'native-corner5-area-contact.cmjsession').read_bytes()
compare_represented(contact,export_session(original,0),True,'corner home player5','area contact')
if contact[408]!=original[0x71] or contact[409]!=original[0x619] or contact[411]!=3:raise ValueError('Corner player5 contact numeric details differ')
original=(a.player3_shot/'corner5-contact-clock.bin').read_bytes()
if (original[0x5f7],original[0x5f8],original[0x5f9])!=(0,0,14):raise ValueError('Unexpected corner added-time boundary')
compare_represented((a.trace/'native-corner5-added-time.cmjsession').read_bytes(),export_session(original,0),False,'corner player5','added time')
for index,name in ((0,'clock'),(1,'presentation'),(2,'next')):
 original=(a.player3_shot/f'corner5-contact-after-{name}.bin').read_bytes()
 compare_represented((a.trace/f'native-corner5-added-resume-{index}.cmjsession').read_bytes(),export_session(original,0),False,'corner player5 added time',name)
original=(a.player3_shot/'corner5-dispatch-8a72-1.bin').read_bytes()
compare_represented((a.trace/'native-corner5-area-dispatch.cmjsession').read_bytes(),export_session(original,0),False,'corner player5','terminal field dispatch')
original=(a.player3_shot/'corner5-dispatch-8c31-1.bin').read_bytes()
if (original[0x441],original[0x5fb],original[0x635],original[0x637])!=(8,0,95,109):raise ValueError('Unexpected corner player5 loose-ball recovery')
compare_represented((a.trace/'native-corner5-area-recovery.cmjsession').read_bytes(),export_session(original,0),False,'corner player5','loose-ball recovery')
for name,pc in (('entry','e0df-1'),('ready','e145-1')):
 original=(a.player3_shot/f'corner5-dispatch-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-corner8-control-{name}.cmjsession').read_bytes(),export_session(original,0),False,'corner player8 control',name)
raw=(a.player3_shot/'corner5-dispatch-e145-1.bin').read_bytes()
for line in (a.trace/'native-added8-journey.txt').read_text().splitlines():
 i,phase,held,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Player8 added-time journey stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 observation,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(held,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-added8-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'player8 added time',str(i))
 if next_phase==4 and (observation['boundary']!='period_end' or (raw[0x441],raw[0x5f7],raw[0x5f8],raw[0x5f9])!=(8,0,0,0)):raise ValueError('Original period-end boundary differs')
for name,pc in (('display','daa0'),('dispatch','c400')):
 original=(a.player3_shot/f'period-transition-30-{pc}-1.bin').read_bytes()
 compare_represented((a.trace/f'native-period-transition-{name}.cmjsession').read_bytes(),export_session(original,0),True,'period transition',name)
original=(a.player3_shot/'period-restart-30-d092.bin').read_bytes()
native=(a.trace/'native-interval-stamina.cmjsession').read_bytes()
for player in range(11):
 original_stamina=original[0x301+12*player]|original[0x302+12*player]<<8
 offset=34+4*player+2;native_stamina=native[offset]|native[offset+1]<<8
 if native_stamina!=original_stamina:raise ValueError(f'Interval stamina differs for player {player}')
restart=(a.player3_shot/'period-restart-30-daaa.bin').read_bytes()
compare_represented((a.trace/'native-interval-restart.cmjsession').read_bytes(),export_session(restart,0),True,'interval restart','initialization')
side=(a.player3_shot/'period-restart-30-dae9.bin').read_bytes()
compare_represented((a.trace/'native-interval-kickoff-side.cmjsession').read_bytes(),export_session(side,0),False,'interval restart','kickoff side')
for name,pc in (('formation','daf5'),('receiver','db0b'),('ball','db0e'),('handoff','db1e')):
 raw=(a.player3_shot/f'period-restart-30-{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-interval-kickoff-{name}.cmjsession').read_bytes(),export_session(raw,0),False,'interval kickoff',name)
ready=(a.player3_shot/'period-next-boundary.bin').read_bytes()
compare_represented((a.trace/'native-interval-kickoff-control.cmjsession').read_bytes(),export_session(ready,0),False,'interval kickoff','control')
for held,pressed in ((0,0),(1,0),(64,64),(128,128)):
 native=(a.trace/f'native-interval18-poll-{held}-{pressed}.cmjsession').read_bytes()
 injected=bytearray(ready);injected[0xe2:0xe4]=native[26:28]
 observation,raw=trace_recovery(rom[16:262160],injected,control_input=(held,pressed))
 if observation['boundary']!='control_poll_wait':raise ValueError('Unexpected interval control poll boundary')
 compare_represented(native,export_session(raw,0),False,'interval player18 poll',str((held,pressed)))
raw=ready
for line in (a.trace/'native-interval18-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Second-half player18 journey stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 observation,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-interval18-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'interval player18 journey',str(i))
from check_ai_field_plan_original import original_choice
unsupported,seed,phase=map(int,(a.trace/'native-interval18-choice.txt').read_text().split())
if unsupported!=-1:raise ValueError('Interval opponent AI choice stopped')
injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255));choice=original_choice(rom[16:262160],injected)
if (choice[0x43b],choice[0x5fc])!=(1,20):raise ValueError('Unexpected original interval pass choice')
compare_represented((a.trace/'native-interval18-choice.cmjsession').read_bytes(),export_session(choice,0),False,'interval player18','AI pass choice')
from check_unopposed_ai_pass_original import observe as observe_unopposed_pass
outcome,checkpoints=observe_unopposed_pass(rom[16:262160],injected)
if outcome!='ready':raise ValueError(f'Unexpected original interval pass outcome {outcome}')
for i,name in ((0,'prepared'),(1,'preclock'),(3,'handoff'),(4,'target'),(5,'flight'),(6,'arrival'),(7,'control'),(8,'ready')):
 native=bytearray((a.trace/f'native-interval18-pass-{i}.cmjsession').read_bytes())
 wanted=bytearray(export_session(checkpoints[name],0))
 if i in (3,4,5,6,7):wanted[298]=native[298]  # acknowledged original presentation facing
 if i==6:native[22:25]=wanted[22:25]  # Flight position is in the separate trajectory until arrival.
 compare_represented(native,wanted,False,'interval player18 pass',name)
line=next(line for line in (a.trace/'native-interval18-pass.txt').read_text().splitlines() if line.startswith('6 '))
values=list(map(int,line.split()));arrival=checkpoints['arrival']
word=lambda address:arrival[address]|arrival[address+1]<<8
if values[6:]!=[word(0x634),word(0x636),arrival[0x5fe]]:raise ValueError('Interval pass arrival trajectory differs')
if (checkpoints['ready'][0x441],checkpoints['ready'][0x635],checkpoints['ready'][0x637])!=(20,128,116):raise ValueError('Unexpected interval pass recipient')
raw=checkpoints['ready']
for line in (a.trace/'native-interval20-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Second-half player20 journey stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 observation,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-interval20-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'interval player20 journey',str(i))
unsupported,seed,phase=map(int,(a.trace/'native-interval20-choice.txt').read_text().split())
if unsupported!=-1:raise ValueError('Interval player20 AI choice stopped')
injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255));choice=original_choice(rom[16:262160],injected)
compare_represented((a.trace/'native-interval20-choice.cmjsession').read_bytes(),export_session(choice,0),False,'interval player20','AI choice')
raw=choice
for line in (a.trace/'native-interval20-next.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Second-half player20 continuation stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 observation,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-interval20-next-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'interval player20 continuation',str(i))
from trace_field_defense_prompt import trace_field_defense_prompt
from trace_area_defense_poll import trace_defense_poll
_,prompt=trace_field_defense_prompt(rom[16:262160],raw)
compare_represented((a.trace/'native-interval20-contact.cmjsession').read_bytes(),export_session(prompt,0),False,'interval player20','defense prompt')
raw=prompt
for line in (a.trace/'native-interval20-defense.txt').read_text().splitlines():
 values=list(map(int,line.split()));i,pressed,stage,cursor,unsupported,seed=values[:6]
 if unsupported!=-1:raise ValueError('Second-half defensive menu stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255))
 observation,raw=trace_defense_poll(rom[16:262160],injected,128 if stage==1 else pressed,preparation=stage==1)
 actual=(a.trace/f'native-interval20-defense-{i}.cmjsession').read_bytes()
 compare_represented(actual,export_session(raw,0),False,'interval player20 defense',str(i))
 expected=[raw[0x61e],raw[0x61f],raw[0x620],*raw[0x60b:0x60f]]
 if values[6:]!=expected:raise ValueError(f'Interval defensive menu state differs at {i}: {values[6:]} != {expected}')
unsupported,seed,phase=map(int,(a.trace/'native-interval20-defended-choice.txt').read_text().split())
if unsupported!=-1:raise ValueError('Interval defended AI choice stopped')
injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255));choice=original_choice(rom[16:262160],injected)
compare_represented((a.trace/'native-interval20-defended-choice.cmjsession').read_bytes(),export_session(choice,0),False,'interval player20 defense','opponent choice')
from check_ai_field_preparation_original import original_preparation
prepared=original_preparation(rom[16:262160],injected)
if int((a.trace/'native-interval20-defended-prepared.txt').read_text().split()[0])!=-1:raise ValueError('Interval defended pass preparation stopped')
compare_represented((a.trace/'native-interval20-defended-prepared.cmjsession').read_bytes(),export_session(prepared,0),False,'interval player20 defense','opponent preparation')
for i,name,options in ((1,'preclock',{'through_clock':True}),
                       (2,'handoff',{'through_handoff':True}),
                       (3,'target',{'through_handoff':True,'through_target':True}),
                       (4,'flight',{'through_handoff':True,'through_target':True,'through_flight':True}),
                       (25,'control',{'through_return':True}),
                       (26,'ready',{'through_return':True,'through_control':True})):
 original=original_preparation(rom[16:262160],injected,**options)
 native=bytearray((a.trace/f'native-interval20-defended-pass-{i}.cmjsession').read_bytes());wanted=bytearray(export_session(original,0))
 if name in ('handoff','target','flight','control'):wanted[298]=native[298]
 compare_represented(native,wanted,False,'interval player20 defended pass',name)
raw=original
for line in (a.trace/'native-interval21-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Second-half player21 journey stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 observation,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-interval21-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'interval player21 journey',str(i))
unsupported,seed,phase=map(int,(a.trace/'native-interval21-choice.txt').read_text().split())
if unsupported!=-1:raise ValueError('Interval player21 AI choice stopped')
injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255));choice=original_choice(rom[16:262160],injected)
compare_represented((a.trace/'native-interval21-choice.cmjsession').read_bytes(),export_session(choice,0),False,'interval player21','AI choice')
outcome,checkpoints=observe_unopposed_pass(rom[16:262160],injected)
if outcome!='ready':raise ValueError(f'Unexpected original player21 pass outcome {outcome}')
for i,name in ((0,'prepared'),(1,'preclock'),(3,'handoff'),(4,'target'),(5,'flight'),(24,'arrival'),(25,'control'),(26,'ready')):
 native=bytearray((a.trace/f'native-interval21-pass-{i}.cmjsession').read_bytes());wanted=bytearray(export_session(checkpoints[name],0))
 if name in ('handoff','target','flight','arrival','control'):wanted[298]=native[298]
 if name=='arrival':native[22:25]=wanted[22:25]
 compare_represented(native,wanted,False,'interval player21 pass',name)
line=next(line for line in (a.trace/'native-interval21-pass.txt').read_text().splitlines() if line.startswith('24 '));values=list(map(int,line.split()));arrival=checkpoints['arrival']
word=lambda address:arrival[address]|arrival[address+1]<<8
if values[6:]!=[word(0x634),word(0x636),arrival[0x5fe]]:raise ValueError('Interval player21 pass arrival trajectory differs')
if (checkpoints['ready'][0x441],checkpoints['ready'][0x635],checkpoints['ready'][0x637])!=(17,98,101):raise ValueError('Unexpected interval player21 pass recipient')
raw=checkpoints['ready']
for line in (a.trace/'native-interval17-journey.txt').read_text().splitlines():
 i,phase,first,second,next_phase,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Second-half player17 journey stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 observation,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-interval17-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'interval player17 journey',str(i))
unsupported,seed,phase=map(int,(a.trace/'native-interval17-choice.txt').read_text().split())
if unsupported!=-1:raise ValueError('Interval player17 AI choice stopped')
injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255));choice=original_choice(rom[16:262160],injected)
if (choice[0x43b],choice[0x5fc],choice[0x638])!=(3,18,88):raise ValueError('Unexpected original player17 action3 choice')
compare_represented((a.trace/'native-interval17-choice.cmjsession').read_bytes(),export_session(choice,0),False,'interval player17','action3 choice')
outcome,action3=observe_unopposed_pass(rom[16:262160],injected,expected_action=3,stop_at_area=True)
if outcome!='return-area':raise ValueError(f'Unexpected original player17 action3 outcome {outcome}')
prepared=action3['prepared']
compare_represented((a.trace/'native-interval17-prepared.cmjsession').read_bytes(),export_session(prepared,0),False,'interval player17','action3 preparation')
clocked=action3['preclock']
compare_represented((a.trace/'native-interval17-clocked.cmjsession').read_bytes(),export_session(clocked,0),False,'interval player17','action3 clock')
compare_represented((a.trace/'native-interval17-action3-flight-2.cmjsession').read_bytes(),export_session(action3['flight'],0),False,'interval player17','action3 flight setup')
line=next(line for line in (a.trace/'native-interval17-action3-flight.txt').read_text().splitlines() if line.startswith('5 '));values=list(map(int,line.split()));arrival=action3['arrival']
word=lambda address:arrival[address]|arrival[address+1]<<8
if values[3:]!=[word(0x634),word(0x636),word(0x639),word(0x63b),arrival[0x5fe]]:raise ValueError('Interval player17 action3 arrival trajectory differs')
compare_represented((a.trace/'native-interval17-action3-swap.cmjsession').read_bytes(),export_session(action3['swap-after'],0),False,'interval player17','action3 swap')
compare_represented((a.trace/'native-interval17-action3-return-1.cmjsession').read_bytes(),export_session(action3['return-flight'],0),False,'interval player17','action3 return flight')
line=next(line for line in (a.trace/'native-interval17-action3-return.txt').read_text().splitlines() if line.startswith('10 '));values=list(map(int,line.split()));arrival=action3['return-arrival']
word=lambda address:arrival[address]|arrival[address+1]<<8
if values[3:]!=[word(0x634),word(0x636),word(0x639),word(0x63b),arrival[0x5fe]]:raise ValueError('Interval player17 action3 return trajectory differs')
compare_represented((a.trace/'native-interval17-action3-received.cmjsession').read_bytes(),export_session(action3['return-area'],0),False,'interval player17','action3 return reception')
from check_keeper_entry_original import observe as observe_keeper_area
area,_,_=observe_keeper_area(rom[16:262160],action3['return-area'],0x8b4a,{0x8b73})
compare_represented((a.trace/'native-interval17-area.cmjsession').read_bytes(),export_session(area,0),False,'interval player17','goal-area setup')
from trace_keeper_area_menu import trace_keeper_area_menu
_,selection=trace_keeper_area_menu(rom[16:262160],area,defense_setup=True)
exportable=bytearray(selection);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-interval17-area-selection.cmjsession').read_bytes(),expected,False,'interval player17','keeper selection')
_,menu=trace_keeper_area_menu(rom[16:262160],area,defense_prompt=True)
for line in (a.trace/'native-interval17-area-menu.txt').read_text().splitlines():
 i,phase,pressed,unsupported,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Interval player17 area menu stopped')
 if phase!=67:
  injected=bytearray(menu);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255))
  observation,menu=trace_defense_poll(rom[16:262160],injected,128 if phase==71 else pressed,preparation=phase==71)
 exportable=bytearray(menu);exportable[0x601]=10
 expected=bytearray(export_session(exportable,0));expected[309]=0
 actual=(a.trace/f'native-interval17-area-menu-{i}.cmjsession').read_bytes()
 compare_represented(actual,expected,False,'interval player17 area menu',str(i))
 actions=list(menu[0x60b:0x60f]);subtypes=[menu[0x606+j] if actions[j]!=255 else 0 for j in range(4)]
 expected_choices=bytes((1,*actions,*subtypes,menu[0x620],menu[0x1e] if phase in (69,70) and pressed==128 and i!=4 else 0))
 if actual[372:383]!=expected_choices:raise ValueError(f'Interval player17 area choices differ at {i}')
from check_generic_goal_area_ai_original import original as original_goal_area_candidate
seed=3787159586;injected=bytearray(menu);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255))
candidate,chosen=original_goal_area_candidate(rom[16:262160],injected,all_choices=True)
if candidate[2]!=0:raise ValueError('Unexpected original player17 goal-area candidate')
if (chosen[0x43b],chosen[0x43c],chosen[0xe2],chosen[0xe3])!=(0,1,34,120):raise ValueError('Unexpected original player17 goal-area shot')
exportable=bytearray(chosen);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-interval17-shot-choice.cmjsession').read_bytes(),expected,False,'interval player17','goal-area shot choice')
_,prepared=original_goal_area_candidate(rom[16:262160],injected,all_choices=True,through_preparation=True)
exportable=bytearray(prepared);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-interval17-shot-prepared.cmjsession').read_bytes(),expected,False,'interval player17','goal-area shot preparation')
from trace_area_shot_original import trace_area_shot
shot_trace=trace_area_shot(rom[16:262160],menu,34,120)
for name,pc in (('clock','83f5'),('field','83f8')):
 raw=shot_trace[f'{pc}-1'];exportable=bytearray(raw);exportable[0x601]=10
 expected=bytearray(export_session(exportable,0));expected[309]=0
 compare_represented((a.trace/f'native-interval17-shot-{name}.cmjsession').read_bytes(),expected,False,'interval player17',f'shot {name}')
for index,pc in ((0,'e773-1'),(2,'e773-2'),(4,'e773-3'),(6,'e773-4'),(7,'e7cc-1')):
 original=shot_trace[pc]
 compare_represented((a.trace/f'native-interval17-shot-flight-{index}.cmjsession').read_bytes(),export_session(original,0),False,'interval player17 shot flight',pc)
original=shot_trace['dd1c-1']
compare_represented((a.trace/'native-interval17-keeper-entry.cmjsession').read_bytes(),export_session(original,0),False,'interval player17','keeper entry')
keeper=menu_trace(rom[16:262160],original,1,3787159586)
for index,name in ((0,'open'),(1,'selected'),(2,'compared')):
 raw=keeper[name];exportable=bytearray(raw);exportable[0x601]=10
 expected=bytearray(export_session(exportable,0));expected[309]=0
 compare_represented((a.trace/f'native-interval17-keeper-menu-{index}.cmjsession').read_bytes(),expected,False,'interval player17 keeper',name)
if keeper['compared'][0x612]!=0:raise ValueError('Unexpected original interval goalkeeper result')
from check_scored_goal_original import observe_scored_goal
goal,_=observe_scored_goal(rom[16:262160],keeper['compared'])
exportable=bytearray(goal);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-interval17-goal.cmjsession').read_bytes(),expected,False,'interval player17','goal registration')
if (goal[0x28],goal[0x29])!=(1,1):raise ValueError('Unexpected original second-half score')
from check_post_goal_original import observe_post_goal
restart=observe_post_goal(rom[16:262160],goal);exportable=bytearray(restart);exportable[0x601]=10
expected=bytearray(export_session(exportable,0));expected[309]=0
compare_represented((a.trace/'native-interval17-goal-restart.cmjsession').read_bytes(),expected,False,'interval player17','goal restart')
from check_human_keeper_menu_original import kickoff_trace
equalizer_kickoff=kickoff_trace(rom[16:262160],restart)
if (equalizer_kickoff['control'][0x5fb],equalizer_kickoff['control'][0x441],equalizer_kickoff['control'][0x635],equalizer_kickoff['control'][0x637])!=(0,5,108,108):raise ValueError('Unexpected original equalizer kickoff control')
for name in ('formation','receiver','ball','handoff','control'):
 raw=equalizer_kickoff[name];exportable=bytearray(raw)
 if exportable[0x600] and exportable[0x601]==0:exportable[0x601]=10
 expected=bytearray(export_session(exportable,0))
 if raw[0x600] and raw[0x601]==0:expected[309]=0
 compare_represented((a.trace/f'native-interval17-goal-{name}.cmjsession').read_bytes(),expected,False,'interval equalizer kickoff',name)
raw=equalizer_kickoff['control']
for line in (a.trace/'native-equalizer5-journey.txt').read_text().splitlines():
 i,phase,held,first,second,next_phase,unsupported=map(int,line.split())
 if unsupported!=-1:raise ValueError('Equalizer player5 journey stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((first,second))
 observation,raw=trace_recovery(rom[16:262160],injected,**{9:{'control_input':(held,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
 compare_represented((a.trace/f'native-equalizer5-journey-{i}.cmjsession').read_bytes(),export_session(raw,0),False,'equalizer player5 journey',str(i))
if observation['boundary']!='field_contact_pending' or (raw[0x441],raw[0x600],raw[0x601])!=(5,1,17):raise ValueError('Unexpected original equalizer contact')
_,_,seed=map(int,(a.trace/'native-equalizer5-contact.txt').read_text().split())
injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255))
observation,raw=trace_menu(rom[16:262160],injected)
if observation['boundary']!='human_action_menu':raise ValueError('Unexpected original equalizer action menu')
compare_represented((a.trace/'native-equalizer5-contact.cmjsession').read_bytes(),export_session(raw,0),False,'equalizer player5','contact menu')
equalizer_opened=raw
for line in (a.trace/'native-equalizer5-choice.txt').read_text().splitlines():
 i,pressed,unsupported,phase,seed=map(int,line.split())
 if unsupported!=-1:raise ValueError('Equalizer player5 pass choice stopped')
 injected=bytearray(raw);injected[0xe2:0xe4]=bytes((seed&255,(seed>>8)&255))
 if i<2:_,raw=trace_menu(rom[16:262160],injected,'poll',pressed)
 else:
  target_seed=bytearray(equalizer_opened);target_seed[0xe2:0xe4]=injected[0xe2:0xe4]
  carrier=0x300+12*5;x=target_seed[carrier+5]|target_seed[carrier+6]<<8;y=target_seed[carrier+7]|target_seed[carrier+8]<<8
  raw=original_target(rom[16:262160],target_seed,x,y,return_ram=True)
 compare_represented((a.trace/f'native-equalizer5-choice-{i}.cmjsession').read_bytes(),export_session(raw,0),i==2,'equalizer player5 choice',str(i))






























print('Matched player9 menu, five defender choices and contacts, flight and player6 recovery:',choices)

