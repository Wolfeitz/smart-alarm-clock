export function createDevice(id, name, owner, time = '07:00') {
  return {id, name, owner, online: true, revision: 1, alarm: {time, days: 'Weekdays', enabled: true}, pending: null};
}
export function propose(device, alarm) {
  if (!/^([01][0-9]|2[0-3]):[0-5][0-9]$/.test(alarm.time) || !['Every day','Weekdays','Weekends'].includes(alarm.days) || typeof alarm.enabled !== 'boolean') throw new Error('Choose a valid alarm time and schedule.');
  if (device.pending) throw new Error('Resolve the pending change first.');
  device.pending = {base: device.revision, alarm: {...alarm}, status: 'pending'};
}
export function acknowledge(device) {
  if (!device.pending || !device.online) return false;
  if (device.pending.base !== device.revision) {device.pending.status = 'conflict'; return false;}
  device.alarm = {...device.pending.alarm}; device.revision++; device.pending = null; return true;
}
export function localEdit(device) {
  device.alarm = {...device.alarm, time: '06:45'}; device.revision++;
}
