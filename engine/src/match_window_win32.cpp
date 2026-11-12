#include "captain/match_client.hpp"
#include "captain/phase_session_io.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace {
using namespace captain;
constexpr int width=1200,height=760;
constexpr int pitch_x=40,pitch_y=150,pitch_w=800,pitch_h=480;
constexpr COLORREF ink=RGB(17,36,48),paper=RGB(245,247,235),grass=RGB(33,101,70);
constexpr COLORREF home=RGB(76,157,227),away=RGB(229,89,85),gold=RGB(250,206,95);

std::wstring widen(const std::string& s) { return std::wstring(s.begin(),s.end()); }
unsigned number(const wchar_t* text,unsigned maximum,const char* message) {
 if(!text || !*text || *text==L'-')throw std::invalid_argument(message);
 wchar_t* end=nullptr;const auto value=std::wcstoull(text,&end,10);
 if(*end || value>maximum)throw std::invalid_argument(message);
 return static_cast<unsigned>(value);
}
std::ifstream open_file(const wchar_t* path,std::ios::openmode mode=std::ios::binary) {
 std::ifstream file(std::filesystem::path(path),mode);
 if(!file)throw std::runtime_error("Cannot open a supplied input file");
 return file;
}
struct App {
 MatchContent content{};
 MatchRuntimeState state{};
 ClientButtons buttons{};
 bool paused=true;
 std::wstring problem{};
};
void fill(HDC dc,int x,int y,int w,int h,COLORREF color) {
 const RECT r{x,y,x+w,y+h};const auto brush=CreateSolidBrush(color);FillRect(dc,&r,brush);DeleteObject(brush);
}
void outline(HDC dc,int x,int y,int w,int h,COLORREF color,int thickness=2) {
 fill(dc,x,y,w,thickness,color);fill(dc,x,y+h-thickness,w,thickness,color);
 fill(dc,x,y,thickness,h,color);fill(dc,x+w-thickness,y,thickness,h,color);
}
void label(HDC dc,const std::wstring& text,int x,int y,int w,int h,int size,COLORREF color,bool bold=false,UINT flags=DT_LEFT|DT_TOP) {
 const auto font=CreateFontW(-size,0,0,0,bold?FW_BOLD:FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
  OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
 const auto previous=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,color);
 RECT r{x,y,x+w,y+h};DrawTextW(dc,text.c_str(),static_cast<int>(text.size()),&r,flags|DT_NOPREFIX);
 SelectObject(dc,previous);DeleteObject(font);
}
void disc(HDC dc,int x,int y,int radius,COLORREF color) {
 const auto brush=CreateSolidBrush(color);const auto old_brush=SelectObject(dc,brush);
 const auto old_pen=SelectObject(dc,GetStockObject(NULL_PEN));Ellipse(dc,x-radius,y-radius,x+radius+1,y+radius+1);
 SelectObject(dc,old_pen);SelectObject(dc,old_brush);DeleteObject(brush);
}
int px(unsigned x){return pitch_x+(static_cast<int>(x)-48)*pitch_w/160;}
int py(unsigned y){return pitch_y+(static_cast<int>(y)-80)*pitch_h/96;}
std::wstring action_name(unsigned action) {
 if(action==0)return L"Shot";if(action==1)return L"Pass";
 if(action==255)return L"Unavailable";return L"Not supported";
}
std::wstring phase_text(const App& app) {
 const auto& s=app.state;const auto phase=s.session.phase;
 if(phase==FieldSessionPhase::human_action_menu)return L"Choose an action";
 if(phase==FieldSessionPhase::ordinary_human_pass_target)return s.target_chooser ? L"Choose a teammate" : L"Choose a pass target";
 if(phase==FieldSessionPhase::field_contact_pending && s.session.current.field.side==11 && s.defense_menu && s.defense_menu->stage==MatchDefenseStage::polling)return L"Choose a defense";
 if(phase==FieldSessionPhase::keeper_presentation_pending && s.session.current.field.side==11 && s.session.action && s.session.action->menu_route==4)return L"Choose a goalkeeper action";
 if(phase==FieldSessionPhase::control_ready)return s.session.current.field.side==0 ? L"Your possession" : L"Opponent possession";
 if(phase==FieldSessionPhase::goal_celebration_pending)return L"Goal";
 if(phase>=FieldSessionPhase::goal_kickoff_pending && phase<=FieldSessionPhase::kickoff_control_pending)return L"Kickoff";
 return L"Play in progress";
}
void render(HDC dc,const App& app) {
 const auto& state=app.state;const auto& field=state.session.current.field;
 fill(dc,0,0,width,height,RGB(232,238,226));
 fill(dc,0,0,width,118,ink);
 label(dc,L"Captain Majid",40,20,440,45,34,paper,true);
 label(dc,L"Provisional development match",42,67,520,28,18,RGB(187,212,198));
 const auto score=state.session.score ? std::to_wstring(state.session.score->left_goals)+L"  :  "+std::to_wstring(state.session.score->right_goals) : L"?  :  ?";
 label(dc,score,630,24,210,50,36,paper,true,DT_CENTER|DT_TOP);
 label(dc,L"Clock  "+std::to_wstring(state.session.current.clock.remaining),900,38,240,40,22,paper,true,DT_RIGHT|DT_TOP);
 fill(dc,pitch_x,pitch_y,pitch_w,pitch_h,grass);
 for(int x=0;x<10;++x)if(x%2==0)fill(dc,pitch_x+x*pitch_w/10,pitch_y,pitch_w/10,pitch_h,RGB(39,113,77));
 outline(dc,pitch_x,pitch_y,pitch_w,pitch_h,paper,3);fill(dc,pitch_x+pitch_w/2-1,pitch_y,3,pitch_h,paper);
 outline(dc,pitch_x,pitch_y+142,115,196,paper,3);outline(dc,pitch_x+pitch_w-115,pitch_y+142,115,196,paper,3);
 const auto old_brush=SelectObject(dc,GetStockObject(NULL_BRUSH));const auto pen=CreatePen(PS_SOLID,3,paper);const auto old_pen=SelectObject(dc,pen);
 Ellipse(dc,pitch_x+pitch_w/2-55,pitch_y+pitch_h/2-55,pitch_x+pitch_w/2+55,pitch_y+pitch_h/2+55);
 SelectObject(dc,old_pen);SelectObject(dc,old_brush);DeleteObject(pen);
 for(unsigned id=1;id<=21;++id){
  if(id==11)continue;const auto& p=field.motion[field_motion_index(static_cast<std::uint8_t>(id))];
  const auto x=p.x>>8,y=p.y>>8;if(x<48 || x>208 || y<80 || y>176)continue;
  const auto sx=px(x),sy=py(y);if(id==field.ball_owner)disc(dc,sx,sy,16,gold);
  disc(dc,sx,sy,12,id<11?home:away);label(dc,std::to_wstring(id),sx-12,sy-9,24,20,12,RGB(255,255,255),true,DT_CENTER|DT_TOP);
 }
 if(state.session.phase==FieldSessionPhase::ordinary_human_pass_target && state.session.human_pass_cursor){
  const auto cell=*state.session.human_pass_cursor;const int x=px((cell/12)*8+52),y=py((cell%12)*8+84);
  outline(dc,x-19,y-19,38,38,gold,3);
 }
 const auto ball=match_ball_view(state);const int bx=px(ball.x),by=py(ball.y);
 if(bx>=pitch_x && bx<=pitch_x+pitch_w && by>=pitch_y && by<=pitch_y+pitch_h){disc(dc,bx,by,6,ink);disc(dc,bx,by,4,paper);}
 label(dc,phase_text(app),880,152,285,36,23,ink,true);
 label(dc,L"Arrow keys  Move / choose\nZ  Confirm\nX  Menu / back\nSpace  Pause",880,205,285,125,18,ink,false,DT_LEFT|DT_TOP|DT_WORDBREAK);
 if(state.session.phase==FieldSessionPhase::human_action_menu && state.session.action && app.content.data->action_menu){
  const auto& action=*state.session.action;const auto route=action.menu_route;
  if(route<app.content.data->action_menu->directions.size()){
   const auto& choices=app.content.data->action_menu->directions[route];
   const wchar_t* names[]={L"Right",L"Left",L"Down",L"Up"};
   for(unsigned i=0;i<4;++i){
    const bool selected=(action.cursor&128u) && (action.cursor&3u)==i;
    label(dc,std::wstring(selected?L">  ":L"   ")+names[i]+L"   "+action_name(choices[i]),880,355+static_cast<int>(i)*29,290,27,17,selected?RGB(147,72,20):ink,selected);
   }
  }
 } else if(state.session.phase==FieldSessionPhase::ordinary_human_pass_target) {
  label(dc,state.target_chooser ? L"Up / down to select a teammate.\nZ confirms, X returns." : L"Move the gold target, then press Z.\nX returns to the action menu.",880,355,280,85,17,ink,false,DT_LEFT|DT_TOP|DT_WORDBREAK);
  if(state.target_chooser && state.session.ordinary_pass_candidates){
   const auto& candidates=state.session.ordinary_pass_candidates->candidates;
   for(unsigned i=0;i<candidates.allied_count && i<candidates.ids.size();++i){const bool selected=i==state.target_chooser->index;
    label(dc,std::wstring(selected?L">  Player ":L"   Player ")+std::to_wstring(candidates.ids[i]),880,446+static_cast<int>(i)*27,270,25,17,selected?RGB(147,72,20):ink,selected);
   }
  }
 } else if(state.session.phase==FieldSessionPhase::keeper_presentation_pending && state.session.current.field.side==11 && state.session.action && state.session.action->menu_route==4 && app.content.data->action_menu && app.content.data->action_menu->keeper_directions) {
  const auto& choices=*app.content.data->action_menu->keeper_directions;const wchar_t* names[]={L"Right",L"Left",L"Down",L"Up"};
  for(unsigned i=0;i<4;++i){const auto action=choices[i];const bool selected=(state.session.action->cursor&128u) && (state.session.action->cursor&3u)==i;
   const auto description=action==0?L"Catch":action==1?L"Punch":L"Unavailable";
   label(dc,std::wstring(selected?L">  ":L"   ")+names[i]+L"   "+description,880,355+static_cast<int>(i)*29,290,27,17,selected?RGB(147,72,20):ink,selected);
  }
 } else if(state.session.phase==FieldSessionPhase::field_contact_pending && state.defense_menu) {
  const auto& menu=*state.defense_menu;
  if(menu.poll.cursor>=menu.defender_count)label(dc,L"Defenders set. Press Z to continue.",880,355,280,60,17,ink,false,DT_LEFT|DT_TOP|DT_WORDBREAK);
  else if(app.content.data->area_defense_menu){
   label(dc,L"Defender "+std::to_wstring(menu.poll.cursor+1)+L" of "+std::to_wstring(menu.defender_count),880,350,280,28,17,ink,true);
   const auto& choices=app.content.data->area_defense_menu->directions[0];const wchar_t* names[]={L"Right",L"Left",L"Down",L"Up"};
   for(unsigned i=0;i<4;++i){const auto action=choices[i];const bool selected=action!=255 && menu.poll.defense.actions[menu.poll.cursor]==action;
    const auto description=action==6?L"Skip":action==255?L"Unavailable":L"Not supported";
    label(dc,std::wstring(selected?L">  ":L"   ")+names[i]+L"   "+description,880,385+static_cast<int>(i)*29,290,27,17,selected?RGB(147,72,20):ink,selected);
   }
   label(dc,L"Z confirms each choice.",880,511,280,50,16,ink);
  }
 }
 label(dc,L"Blue: home    Red: opponent",40,654,600,24,17,ink);
 label(dc,L"Placeholder view  /  private development snapshot",40,688,780,24,16,RGB(71,94,82));
 if(app.paused){
  fill(dc,288,292,520,158,ink);outline(dc,288,292,520,158,gold,3);
  label(dc,app.problem.empty()?L"Paused":L"Play stopped",312,309,470,38,27,paper,true);
  label(dc,app.problem.empty()?L"Press Space to continue.":app.problem,312,354,470,80,18,paper,false,DT_LEFT|DT_TOP|DT_WORDBREAK);
 }
}
void render_check(App& app,const std::filesystem::path& destination) {
 if(std::filesystem::exists(destination))throw std::runtime_error("Render check destination already exists");
 BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;
 info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
 const auto dc=CreateCompatibleDC(nullptr);void* pixels=nullptr;
 const auto bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);
 if(!dc || !bitmap || !pixels){if(bitmap)DeleteObject(bitmap);if(dc)DeleteDC(dc);throw std::runtime_error("Cannot create render check image");}
 const auto previous=SelectObject(dc,bitmap);app.paused=false;render(dc,app);GdiFlush();
 BITMAPFILEHEADER file{};file.bfType=0x4d42;file.bfOffBits=sizeof(file)+sizeof(info.bmiHeader);file.bfSize=file.bfOffBits+width*height*4;
 std::ofstream output(destination,std::ios::binary);output.write(reinterpret_cast<const char*>(&file),sizeof(file));
 output.write(reinterpret_cast<const char*>(&info.bmiHeader),sizeof(info.bmiHeader));output.write(static_cast<const char*>(pixels),width*height*4);
 output.close();SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);
 if(!output)throw std::runtime_error("Cannot write render check image");
}
std::uint8_t key_mask(WPARAM key) {
 switch(key){case VK_RIGHT:return 1;case VK_LEFT:return 2;case VK_DOWN:return 4;case VK_UP:return 8;case 'X':return 64;case 'Z':return 128;default:return 0;}
}
LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM wparam,LPARAM lparam) {
 auto* app=reinterpret_cast<App*>(GetWindowLongPtrW(window,GWLP_USERDATA));
 if(message==WM_NCCREATE){app=static_cast<App*>(reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams);SetWindowLongPtrW(window,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(app));}
 if(!app)return DefWindowProcW(window,message,wparam,lparam);
 switch(message){
 case WM_CREATE:SetTimer(window,1,50,nullptr);return 0;
 case WM_SIZE:InvalidateRect(window,nullptr,FALSE);return 0;
 case WM_KILLFOCUS:app->buttons.clear();app->paused=true;InvalidateRect(window,nullptr,FALSE);return 0;
 case WM_KEYDOWN:
  if(wparam==VK_ESCAPE){DestroyWindow(window);return 0;}
  if(wparam==VK_SPACE && !(lparam&(1LL<<30))){app->paused=!app->paused;app->buttons.clear();if(!app->paused)app->problem.clear();InvalidateRect(window,nullptr,FALSE);return 0;}
  if(!app->paused)app->buttons.press(key_mask(wparam));return 0;
 case WM_KEYUP:app->buttons.release(key_mask(wparam));return 0;
 case WM_TIMER:
  if(!app->paused){
   try {
    const auto keys=app->buttons.take();const auto input=client_input(app->state,keys.held,keys.pressed);
    auto result=std::make_unique<MatchStepResult>(step_match(app->state,input,app->content));
    if(result->unsupported){app->paused=true;app->buttons.clear();app->problem=L"Unsupported: "+widen(capability_name(*result->unsupported))+L". The previous state is preserved.";}
    else app->state=std::move(result->state);
   } catch(const std::exception& error){app->paused=true;app->buttons.clear();app->problem=L"Stopped: "+widen(error.what());}
   InvalidateRect(window,nullptr,FALSE);
  }
  return 0;
 case WM_ERASEBKGND:return 1;
 case WM_PAINT:{
  PAINTSTRUCT paint{};const auto screen=BeginPaint(window,&paint);RECT client{};GetClientRect(window,&client);
  const int w=client.right,h=client.bottom;if(w>0 && h>0){
   const auto memory=CreateCompatibleDC(screen);const auto bitmap=CreateCompatibleBitmap(screen,w,h);const auto old_bitmap=SelectObject(memory,bitmap);
   RECT area{0,0,w,h};FillRect(memory,&area,static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
   const int view_w=std::min(w,h*width/height),view_h=std::min(h,w*height/width);
   SetMapMode(memory,MM_ANISOTROPIC);SetWindowExtEx(memory,width,height,nullptr);SetViewportExtEx(memory,view_w,view_h,nullptr);SetViewportOrgEx(memory,(w-view_w)/2,(h-view_h)/2,nullptr);
   render(memory,*app);SetMapMode(memory,MM_TEXT);SetViewportOrgEx(memory,0,0,nullptr);BitBlt(screen,0,0,w,h,memory,0,0,SRCCOPY);
   SelectObject(memory,old_bitmap);DeleteObject(bitmap);DeleteDC(memory);
  }EndPaint(window,&paint);return 0;
 }
 case WM_DESTROY:KillTimer(window,1);PostQuitMessage(0);return 0;
 default:return DefWindowProcW(window,message,wparam,lparam);
 }
}
}

int WINAPI WinMain(HINSTANCE instance,HINSTANCE,LPSTR,int show) {
 int argc=0;auto argv=CommandLineToArgvW(GetCommandLineW(),&argc);
 if(!argv)return 1;
 try {
  if(argc<3)throw std::invalid_argument("Usage: captain-match-window PACK SNAPSHOT --distance FILE --velocity FILE [--seed N] [--ball-fractions X Y] [--retained-defense-action N] [--score LEFT RIGHT KICKOFF]");
  auto app=std::make_unique<App>();MatchConfig config{};config.mode=MatchTimingMode::provisional_demo;config.seed=1;
  auto pack=open_file(argv[1]);app->content.data=load_shot_content(pack);
  auto snapshot=open_file(argv[2]);config.initial=load_phase_session(snapshot);
  bool have_distance=false,have_velocity=false,have_seed=false,have_score=false;std::optional<std::filesystem::path> render_output;
  for(int i=3;i<argc;){const std::wstring option=argv[i++];
   if(option==L"--distance" && i<argc && !have_distance){auto file=open_file(argv[i++]);app->content.pass_distance=read_pass_distance_content(file);have_distance=true;}
   else if(option==L"--velocity" && i<argc && !have_velocity){auto file=open_file(argv[i++],std::ios::in);app->content.ball_velocity=read_ball_velocity_content(file);have_velocity=true;}
   else if(option==L"--seed" && i<argc && !have_seed){config.seed=number(argv[i++],0xffffffffu,"Seed must be 1..4294967295");have_seed=true;}
   else if(option==L"--ball-fractions" && i+1<argc && !config.ball_fraction){const auto x=number(argv[i++],255,"Fraction must be 0..255");const auto y=number(argv[i++],255,"Fraction must be 0..255");config.ball_fraction=MatchBallFraction{static_cast<std::uint8_t>(x),static_cast<std::uint8_t>(y)};}
   else if(option==L"--retained-defense-action" && i<argc && !config.retained_defense_action)config.retained_defense_action=static_cast<std::uint8_t>(number(argv[i++],6,"Retained action must be 0..6"));
   else if(option==L"--score" && i+2<argc && !have_score){
    if(config.initial->score)throw std::invalid_argument("Snapshot already contains score metadata");
    const auto left=number(argv[i++],255,"Score bytes must be 0..255");const auto right=number(argv[i++],255,"Score bytes must be 0..255");const auto kickoff=number(argv[i++],255,"Kickoff byte must be 0..255");
    config.initial->score=MatchScoreContext{static_cast<std::uint8_t>(left),static_cast<std::uint8_t>(right),static_cast<std::uint8_t>(kickoff)};have_score=true;
   }
   else if(option==L"--render-check" && i<argc && !render_output)render_output=std::filesystem::path(argv[i++]);
   else throw std::invalid_argument("Invalid, incomplete or repeated option");
  }
  if(!have_distance || !have_velocity || !config.seed)throw std::invalid_argument("Distance, velocity and a nonzero seed are required");
  auto start=std::make_unique<MatchStepResult>(create_match(config,app->content));
  if(start->unsupported)throw std::runtime_error(std::string("Cannot start: ")+capability_name(*start->unsupported));
  app->state=std::move(start->state);
  if(render_output){render_check(*app,*render_output);LocalFree(argv);return 0;}
  WNDCLASSW type{};type.lpfnWndProc=window_proc;type.hInstance=instance;type.hCursor=LoadCursor(nullptr,IDC_ARROW);type.lpszClassName=L"CaptainMajidMatchWindow";
  if(!RegisterClassW(&type))throw std::runtime_error("Cannot register window class");
  const auto window=CreateWindowExW(0,type.lpszClassName,L"Captain Majid - Provisional development match",WS_OVERLAPPEDWINDOW,
   CW_USEDEFAULT,CW_USEDEFAULT,1216,799,nullptr,nullptr,instance,app.get());
  if(!window)throw std::runtime_error("Cannot create match window");
  ShowWindow(window,show);UpdateWindow(window);MSG message{};
  while(GetMessageW(&message,nullptr,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}
  LocalFree(argv);return static_cast<int>(message.wParam);
 } catch(const std::exception& error){const auto message=widen(error.what());MessageBoxW(nullptr,message.c_str(),L"Captain Majid development window",MB_OK|MB_ICONERROR);LocalFree(argv);return 1;}
}
