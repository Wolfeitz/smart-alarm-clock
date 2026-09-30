import {createDevice, propose, acknowledge, localEdit} from './model.mjs';
const $ = selector => document.querySelector(selector);
const titles = {today:'Today',devices:'People & devices',alarms:'Alarms & routines',music:'Music',assistant:'Companion',connections:'Connections',privacy:'Privacy & sharing'};
let devices, profile='rob', page='today', sharing, toastTimer;
const name = () => profile==='rob'?'Rob':'Alex';
const device = () => devices[profile];
function reset(){devices={rob:createDevice('bedroom','Bedside','rob'),alex:createDevice('studio','Studio clock','alex','07:30')};sharing={rob:false,alex:false};}
reset();
function timeParts(value){const [h,m]=value.split(':'); return {time:(Number(h)%12||12)+':'+m,period:Number(h)<12?'AM':'PM'};}
function timeLabel(value){const t=timeParts(value);return t.time+' '+t.period;}
function notice(message){$('#toast').textContent=message;$('#toast').style.display='block';clearTimeout(toastTimer);toastTimer=setTimeout(()=>$('#toast').style.display='none',4000);}
function intro(title,sub){return '<div class="intro"><div><h1>'+title+'</h1><p>'+sub+'</p></div><span class="date-chip">Wednesday, September 30 · sample day</span></div>';}
function pendingCard(d){
 if(!d.pending)return '';
 const conflict=d.pending.status==='conflict';
 return '<div class="pending" role="status"><strong>'+(conflict?'The clock changed while you were editing.':'Waiting for the bedside clock')+'</strong>'+
 (conflict?'Your website change has not replaced the newer device setting.':'Requested '+timeLabel(d.pending.alarm.time)+'. The saved alarm remains '+timeLabel(d.alarm.time)+'.')+
 '<div class="row"><span>'+(d.online?'Demo device online':'Demo device offline')+'</span>'+
 (!conflict?'<button class="secondary" data-action="ack" '+(!d.online?'disabled':'')+'>Simulate device acknowledgment</button>':'')+
 '<button class="text-button" data-action="discard">Discard pending change</button></div></div>';
}
function deviceCard(d){
 return '<article class="card device-card"><div class="card-head"><h3>'+name()+'’s '+d.name.toLowerCase()+'</h3><span class="chip '+(!d.online?'offline':'')+'">'+(d.online?'● Demo online':'○ Demo offline')+'</span></div>'+
 '<div class="device" aria-label="Concept bedside display"><div class="device-top"><span>WEDNESDAY, SEP 30</span><span>☀ &nbsp;72° · demo</span></div><div class="device-clock">6:42<small>AM</small></div><div class="device-date">A little room before the day begins.</div><div class="device-alarm"><span>◴ &nbsp;'+(d.alarm.enabled?'Wake-up '+timeLabel(d.alarm.time):'No alarm enabled')+'</span><span>'+d.alarm.days+'</span></div><div class="device-bottom"><span>◷ &nbsp; Clock</span><span>◴ &nbsp; Alarms</span><span>♫ &nbsp; Music</span><span>✧ &nbsp; Companion</span></div></div>'+
 '<div class="device-caption"><span>Device screen concept · applied settings</span><span>Revision '+d.revision+'</span></div></article>';
}
function alarmCard(d){
 const t=timeParts(d.alarm.time);
 return '<article class="card alarm-card"><div class="row"><span class="eyebrow">YOUR NEXT WAKE-UP</span><span class="chip">'+(d.alarm.enabled?'Enabled':'Off')+'</span></div><div class="big-time">'+t.time+' <small>'+t.period+'</small></div><div class="alarm-meta">'+d.alarm.days+' · '+d.name+' · local tone</div><div class="row"><span style="font-size:11px;color:#796b54">Saved on demo device</span><button class="secondary" data-action="edit" '+(d.pending?'disabled':'')+'>Edit alarm ↗</button></div></article>';
}
function assistantCard(){return '<article class="card assistant-card"><div class="pet" aria-hidden="true"></div><div><span class="eyebrow">A LITTLE COMPANY</span><h3>Your companion, your way.</h3><p>Morning briefings, helpful reminders, a familiar face.</p><button class="text-button" data-page="assistant">Explore the concept →</button></div></article>';}
function demoTools(d){return '<details class="demo-tools"><summary>Prototype controls · explore offline and conflict behavior</summary><div class="row"><button class="secondary" data-action="online">'+(d.online?'Take demo device offline':'Bring demo device online')+'</button><button class="secondary" data-action="local">Simulate local edit to 6:45 AM</button></div><p>These controls affect simulated state only. Refreshing resets all demo changes.</p></details>';}
function today(d){return intro('Good morning, '+name()+'.','Your day, your people, a calmer place to begin.')+pendingCard(d)+
 '<div class="grid">'+deviceCard(d)+'<div class="stack">'+alarmCard(d)+assistantCard()+'</div></div>'+
 '<div class="section-title"><h2>Make yourself at home</h2><span>Only the things you choose to connect.</span></div><div class="cards-three">'+
 '<article class="card small-card"><div class="symbol">♫</div><h3>A better way to wake up</h3><p>Choose a room speaker and the sound of your morning.</p><button class="text-button" data-page="music">Set up sound →</button></article>'+
 '<article class="card small-card"><div class="symbol">⌁</div><h3>Your health, your space</h3><p>Glucose stays private. You choose who sees it and where.</p><button class="text-button" data-page="privacy">Review privacy →</button></article>'+
 '<article class="card small-card"><div class="symbol">⌂</div><h3>Room for everyone</h3><p>Personal settings for each person, on their own device.</p><button class="text-button" data-page="devices">See your household →</button></article></div>'+demoTools(d);
}
function people(){return intro('A home for every person.','Personal devices. Shared spaces. Everyone keeps their own settings.')+
 '<article class="card large-card"><div class="card-head"><h3>Your demo household</h3><span class="chip">2 people · 2 devices</span></div>'+
 Object.entries(devices).map(([id,d])=>'<div class="member"><span class="avatar">'+(id==='rob'?'R':'A')+'</span><div class="grow"><h3>'+(id==='rob'?'Rob':'Alex · fictional family member')+'</h3><p>'+d.name+' · '+timeLabel(d.alarm.time)+' '+d.alarm.days+'</p></div><button class="secondary" data-profile="'+id+'">View '+(id==='rob'?'Rob':'Alex')+'’s space</button></div>').join('')+
 '<div class="note">Pairing concept: your clock shows a short-lived code. Enter it here, choose its person or room, and confirm on the clock. Real pairing is not connected in this prototype.</div></article>';
}
function music(){return intro('Wake up to something better.','A room speaker for music, mornings and everything in between.')+
 '<article class="card large-card"><div class="empty"><span class="chip offline">No speaker connected</span><h2>Make your speaker part of the morning.</h2><p>Choose a friendly room name, an alarm volume and a favorite sound. The clock should handle the details.</p></div>'+
 '<label class="field">Explore a speaker option<select id="speaker-choice"><option>Existing Sonos speaker</option><option>Another network speaker</option><option>Louder local speaker</option></select></label>'+
 '<p id="speaker-detail" class="muted">Sonos through your existing Home Assistant is the first proposed network route. No speaker or server is connected here.</p>'+
 '<div class="note">The board’s current tone is too quiet for the requested wake-up experience. A network speaker improves sound, but a loud offline backup still needs a local hardware solution.</div><button class="secondary" data-page="connections">View connection plan</button></article>';
}
function assistant(){return intro('A familiar presence.','Useful when you need it. Quiet when you don’t.')+
 '<article class="card large-card"><div class="assistant-card card"><div class="pet" aria-hidden="true"></div><div><h2>Meet your future companion.</h2><p>Illustrative character only · ChatGPT and Dot are not connected.</p></div></div><div class="connection"><h3>“Set my weekday alarm for seven.”</h3><p>Proposed assistant actions use the same permissions and device confirmation as the website. A reply is not proof the clock saved it.</p><button class="secondary" data-action="edit" '+(device().pending?'disabled':'')+'>Try the alarm editing flow</button></div><div class="connection"><h3>A briefing that belongs to you</h3><p>Weather, your schedule and chosen reminders. Private health information is excluded unless separately authorized.</p></div><div class="connection"><h3>Your Dot and your device</h3><p>The supported plugin route is under investigation. This concept does not represent a live Dot session, memory or activity feed.</p></div></article>';
}
function privacy(){return intro('Private means yours.','Sharing a home should not mean sharing everything.')+
 '<article class="card large-card"><div class="card-head"><h3>'+name()+'’s glucose card</h3><span class="chip">Libre not connected</span></div><label class="check"><input id="share-health" type="checkbox" '+(sharing[profile]?'checked':'')+'> Preview sharing fictional readings with the other demo member</label>'+
 '<div class="note">This changes a visual demonstration only. It grants no real access. Each person’s sharing choice is separate.</div>'+
 (sharing[profile]?'<div class="card"><span class="eyebrow">SYNTHETIC EXAMPLE · NOT LIVE</span><div class="privacy-value">108 <small>mg/dL &nbsp; →</small></div><p class="muted">Sample timestamp: 6:38 AM · stale example<br>A real reading must always show its age and source.</p></div>':'<div class="empty"><h2>Only you, by default.</h2><p>No glucose readings are shown to other family members. Household administration does not grant access to private health data.</p></div>')+
 '<p class="muted">Libre would be a supplementary display. The manufacturer’s app and alarms remain the primary monitoring path. Assistant access would require a separate choice.</p></article>';
}
function connections(){return intro('Connect what matters.','A useful clock on its own. More personal with your chosen connections.')+
 '<article class="card large-card">'+[
 ['Home Assistant','Optional controls and speakers through your existing server. No installation on the Pi is performed.'],
 ['ChatGPT / Dot','Proposed authenticated tools for your household. Direct embedded Dot access is not verified.'],
 ['Libre','Read-only value, trend and freshness. Data access and per-person sharing still need qualification.'],
 ['Music & speakers','Speaker selection, saved favorites and alarm-specific volume. Real playback remains to be verified.']
 ].map(([title,description])=>'<div class="connection"><div class="row"><h3>'+title+'</h3><span class="chip offline">Not connected</span></div><p>'+description+'</p></div>').join('')+'</article>';
}
function render(){
 const d=device();$('#page-title').textContent=titles[page];$('#profile').value=profile;
 document.querySelectorAll('[data-page]').forEach(b=>{b.classList.toggle('active',b.dataset.page===page);if(b.closest('nav'))b.setAttribute('aria-current',b.dataset.page===page?'page':'false');});
 const views={today:()=>today(d),devices:people,alarms:()=>intro('A gentler start.','Set it here. Keep it on the clock, even when the network goes away.')+pendingCard(d)+'<div class="grid">'+alarmCard(d)+deviceCard(d)+'</div>'+demoTools(d),music,assistant,privacy,connections};
 $('#content').innerHTML=views[page]();
}
function navigate(next){page=next in titles?next:'today';location.hash=page;render();}
function edit(){const d=device();if(d.pending)return;$('#alarm-device').textContent=name()+' · '+d.name;$('#alarm-time').value=d.alarm.time;$('#alarm-days').value=d.alarm.days;$('#alarm-enabled').checked=d.alarm.enabled;$('#form-error').textContent='';$('#alarm-dialog').showModal();}
document.addEventListener('click',e=>{
 const b=e.target.closest('button');if(!b)return;
 if(b.dataset.page){navigate(b.dataset.page);return;}
 if(b.dataset.profile){profile=b.dataset.profile;navigate('today');return;}
 const d=device();
 switch(b.dataset.action){
  case 'edit':edit();break;
  case 'online':d.online=!d.online;render();break;
  case 'local':localEdit(d);render();notice('Demo clock changed locally to 6:45 AM.');break;
  case 'ack':const applied=acknowledge(d);render();notice(applied?'Saved on the demo device.':'Conflict: review the newer device settings.');break;
  case 'discard':d.pending=null;render();notice('Pending demo change discarded.');break;
 }
});
$('#profile').addEventListener('change',e=>{profile=e.target.value;render();});
$('#content').addEventListener('change',e=>{
 if(e.target.id==='share-health'){sharing[profile]=e.target.checked;render();}
 if(e.target.id==='speaker-choice')$('#speaker-detail').textContent=[
 'Sonos through your existing Home Assistant is the first proposed network route. No speaker or server is connected here.',
 'Compatibility depends on the actual model. We will verify its control, volume and playback behavior before integrating.',
 'A suitably matched speaker/amplifier is needed for louder sound without the network. Hardware compatibility remains to be verified.'
 ][e.target.selectedIndex];
});
$('#alarm-form').addEventListener('submit',e=>{e.preventDefault();try{propose(device(),{time:$('#alarm-time').value,days:$('#alarm-days').value,enabled:$('#alarm-enabled').checked});$('#alarm-dialog').close();navigate('alarms');notice('Demo change pending. Current device alarm is unchanged.');}catch(error){$('#form-error').textContent=error.message;}});
for(const id of ['close-dialog','cancel-dialog'])$('#'+id).addEventListener('click',()=>$('#alarm-dialog').close());
$('#reset-demo').addEventListener('click',()=>{reset();profile='rob';navigate('today');notice('Demo reset. No real device was changed.');});
window.addEventListener('hashchange',()=>{page=location.hash.slice(1) in titles?location.hash.slice(1):'today';render();});
page=location.hash.slice(1) in titles?location.hash.slice(1):'today';render();
