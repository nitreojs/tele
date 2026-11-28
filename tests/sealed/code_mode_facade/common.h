#pragma once
#include <QtCore/QtCore>
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <vector>
#include <compare>

template<typename T> using not_null = T;
template<typename T> using Fn = std::function<T>;
inline QString operator""_q(const char16_t *text, size_t size) { return QString::fromUtf16(text, qsizetype(size)); }
namespace crl { using time = qint64; inline time clock = 1000; inline time now() { return clock; } inline std::vector<std::function<void()>> mainQueue; template<typename T,typename F> void on_main(T*,F f){mainQueue.emplace_back(std::move(f));} inline void drain(){auto work=std::move(mainQueue);mainQueue.clear();for(auto f:work)f();} }
namespace rpl {
struct lifetime {
 std::shared_ptr<bool> alive = std::make_shared<bool>(true);
 std::vector<std::function<void()>> cleanup;
 ~lifetime() { *alive=false; for(auto i=cleanup.rbegin();i!=cleanup.rend();++i)(*i)(); }
 template<typename T, typename... A> T* make_state(A&&... a) { auto p=new T(std::forward<A>(a)...); cleanup.push_back([=]{delete p;}); return p; }
};
template<typename T=void> struct producer;
template<typename T> struct producer { std::shared_ptr<std::vector<std::function<void(T)>>> handlers=std::make_shared<std::vector<std::function<void(T)>>>(); };
template<> struct producer<void> { std::shared_ptr<std::vector<std::function<void()>>> handlers=std::make_shared<std::vector<std::function<void()>>>(); };
template<typename T=void> struct event_stream { producer<T> value; producer<T> events() {return value;} void fire(T v){auto copy=*value.handlers; for(auto f:copy)f(v);} };
template<> struct event_stream<void> { producer<> value; producer<> events(){return value;} void fire(){auto copy=*value.handlers;for(auto f:copy)f();} };
template<typename F> struct Next {F f; std::shared_ptr<bool> alive;};
template<typename F> Next<F> on_next(F f,lifetime& l){return {std::move(f),l.alive};}
template<typename T,typename F> void operator|(producer<T> p,Next<F> n){p.handlers->push_back([n](T v){if(*n.alive){if constexpr(std::is_invocable_v<F,T>)n.f(v);else n.f();}});}
template<typename F> void operator|(producer<void> p,Next<F> n){p.handlers->push_back([n]{if(*n.alive)n.f();});}
template<typename T> producer<T> single(T){return {};}
}
namespace base {
template<typename T>struct weak {T* value;T* get()const{return value;}T* operator->()const{return value;}explicit operator bool()const{return value!=nullptr;}};template<typename T>weak<T> make_weak(T* p){return {p};}
struct Timer { std::function<void()> callback; crl::time due=0; void setCallback(std::function<void()> f){callback=std::move(f);} void cancel(){due=0;} void callOnce(crl::time t){due=crl::now()+t;} void fire(){if(callback)callback();} };
namespace options { template<typename T> struct option { T v{};rpl::event_stream<> changed; T value(){return v;} void set(T value){v=value;changed.fire();} auto changes(){return changed.events();} }; template<typename T> option<T>& lookup(const char* name){static std::map<std::string,option<T>> all; return all[name];} }
}
struct PeerId {quint64 value=0;auto operator<=>(const PeerId&) const=default;};
struct MsgId {qint64 bare=0;auto operator<=>(const MsgId&) const=default;};
struct FullMsgId {PeerId peer;MsgId msg;auto operator<=>(const FullMsgId&) const=default;};
enum class EntityType {Url,CustomUrl,Code,Pre,Bold};
struct EntityInText {EntityType t;int o,l;QString d;EntityInText(EntityType t,int o,int l,QString d={}):t(t),o(o),l(l),d(std::move(d)){} EntityType type()const{return t;} int offset()const{return o;} int length()const{return l;}QString data()const{return d;}bool validForText(int n)const{return o>=0&&l>0&&o<=n&&l<=n-o;}};
struct TextWithEntities {QString text;QVector<EntityInText> entities;};
class HistoryItem;
class History;
namespace Main {class Session;class Account;class Domain;}
struct PeerData {inline static constexpr PeerId kServiceNotificationsId{777000};PeerId id;};
namespace Data {struct Session {Main::Session* owner;std::map<FullMsgId,HistoryItem*> messages;History* service=nullptr;rpl::event_stream<const HistoryItem*> removed;rpl::event_stream<> cleared;int dependencies=0;HistoryItem* message(FullMsgId id){auto i=messages.find(id);return i==messages.end()?nullptr:i->second;}History* historyLoaded(PeerId){return service;}auto itemRemoved(){return removed.events();}auto sessionDataAboutToBeCleared(){return cleared.events();}void updateDependentMessages(HistoryItem*){++dependencies;}};}
namespace Main {
class Account {public:rpl::event_stream<Session*> changes;auto sessionChanges(){return changes.events();}};
class Session {public:Data::Session dataValue{this};rpl::lifetime life;Account acc;quint64 id=42;Data::Session& data(){return dataValue;}rpl::lifetime& lifetime(){return life;}quint64 uniqueId(){return id;}Account& account(){return acc;}};
class Domain {public:rpl::event_stream<Session*> changes;auto activeSessionChanges(){return changes.events();}};
}
class History {public:PeerData* peer;Main::Session* s;Main::Session& session(){return *s;}};
namespace Tele {TextWithEntities FilterSealedCodeText(HistoryItem*,TextWithEntities);}
class HistoryItem {public:History* h;FullMsgId id;bool outgoing=false;bool service=false;TextWithEntities value;int invalidated=0;History* history(){return h;}bool out(){return outgoing;}bool isService(){return service;}FullMsgId fullId()const{return id;}const TextWithEntities& originalText(){return value;}void setText(TextWithEntities t){value=Tele::FilterSealedCodeText(this,std::move(t));}void invalidateChatListEntry(){++invalidated;}};
namespace Core {struct Notifications {int cleared=0;void clearFromHistory(History*){++cleared;}};struct AppState {Main::Domain domainValue;Notifications notificationsValue;rpl::event_stream<bool> lockEvents;bool locked=false;bool passcodeLocked(){return locked;}auto passcodeLockChanges(){return lockEvents.events();}Notifications& notifications(){return notificationsValue;}Main::Domain& domain(){return domainValue;}};inline AppState& App(){static AppState value;return value;}}
namespace style {
struct icon {};
struct margins {int l=0,t=0,r=0,b=0;int left()const{return l;}int right()const{return r;}};
enum align {al_justify};
}
namespace st {inline int boxWideWidth=400,boxLabel=0,defaultInputField=0,boxMaxListHeight=492,settingsButton=0;inline style::margins boxPadding{24,14,24,8};inline style::icon menuIconRestore;}
template<typename T> class object_ptr:public std::unique_ptr<T>{public:template<typename...A>object_ptr(A&&...a):std::unique_ptr<T>(new T(std::forward<A>(a)...)){} };
namespace Ui {
struct FlatLabel {template<typename...A>FlatLabel(A&&...){} };
struct InputField {template<typename...A>InputField(A&&...){}void setMaxLength(int){}QString getLastText(){return {};}void showError(){}void setFocusFast(){} };
struct SettingsButton {template<typename...A>SettingsButton(A&&...){}void setFocusPolicy(Qt::FocusPolicy){}void setAllowTextLines(int=0){}void setClickedCallback(Fn<void()>){}};
struct GenericBox {rpl::lifetime life;rpl::event_stream<> closing;void setTitle(rpl::producer<QString>){}void setWidth(int){}void setMaxHeight(int){}template<typename T,typename...A>T* addRow(object_ptr<T> w,A&&...){return w.release();}void addButton(rpl::producer<QString>,Fn<void()> f){}void closeBox(){}auto boxClosing(){return closing.events();}rpl::lifetime& lifetime(){return life;}void setFocusCallback(Fn<void()>){}};
}
namespace Settings {struct IconDescriptor {const style::icon* icon=nullptr;};inline object_ptr<Ui::SettingsButton> CreateButtonWithIcon(Ui::GenericBox* box,rpl::producer<QString> text,int st,IconDescriptor){return object_ptr<Ui::SettingsButton>(box,std::move(text),st);}}
namespace Window {class SessionController {public:Main::Session* value;Main::Session& session(){return *value;}template<typename T>void show(T){}void showToast(QString){} };}
template<typename...A>int Box(A&&...){return 0;}
namespace Tele {
inline const char kOptionSealedDecoyCodes[]="codes";inline const char kOptionSealedDecoyHours[]="hours";
inline bool SealedDecoyCodes(){return base::options::lookup<bool>(kOptionSealedDecoyCodes).value();}
inline QString SealedDecoyHours(){const auto v=base::options::lookup<QString>(kOptionSealedDecoyHours).value();return v.isEmpty()?QStringLiteral("24"):v;}
inline void SetSealedDecoyHours(const QString& v){base::options::lookup<QString>(kOptionSealedDecoyHours).set(v);}
inline QString Lower(QString v){return v;}
inline void RequireSealedOrdinaryPasscode(Window::SessionController*,Fn<void()>){ }
inline QString TestRoot;inline QString SealedDuressRoot(){return TestRoot;}inline QByteArray SealedDuressContext(){return QByteArray("synthetic test profile");}
}
