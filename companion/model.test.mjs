import {test} from 'node:test';
import assert from 'node:assert/strict';
import {createDevice, propose, acknowledge, localEdit} from './model.mjs';
test('offline proposal leaves applied alarm unchanged until online acknowledgment',()=>{
 const d=createDevice('a','Bedside','rob');d.online=false;
 propose(d,{time:'08:00',days:'Weekends',enabled:true});
 assert.equal(d.alarm.time,'07:00');assert.equal(acknowledge(d),false);
 assert.ok(d.pending);d.online=true;assert.equal(acknowledge(d),true);
 assert.equal(d.alarm.time,'08:00');assert.equal(d.pending,null);
 assert.equal(d.revision,2);assert.equal(acknowledge(d),false);
});
test('local edits reject stale web changes rather than overwrite them',()=>{
 const d=createDevice('a','Bedside','rob');
 propose(d,{time:'08:00',days:'Weekends',enabled:true});localEdit(d);
 assert.equal(acknowledge(d),false);assert.equal(d.alarm.time,'06:45');
 assert.equal(d.pending.status,'conflict');
});
test('profiles remain isolated and invalid/duplicate requests are rejected',()=>{
 const a=createDevice('a','A','rob'),b=createDevice('b','B','alex');
 assert.throws(()=>propose(a,{time:'25:30',days:'Every day',enabled:true}));
 propose(a,{time:'09:00',days:'Every day',enabled:false});
 assert.throws(()=>propose(a,{time:'10:00',days:'Every day',enabled:true}));
 acknowledge(a);assert.equal(b.alarm.time,'07:00');assert.equal(b.alarm.enabled,true);
});
