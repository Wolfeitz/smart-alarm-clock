import assert from 'node:assert/strict';
import {mkdir} from 'node:fs/promises';
const {chromium}=await import(process.env.PLAYWRIGHT_MODULE || 'playwright');
const output=process.env.PREVIEW_OUTPUT || '/tmp/esp-link-companion';
await mkdir(output,{recursive:true});
const browser=await chromium.launch({executablePath:process.env.CHROMIUM || '/usr/bin/chromium',headless:true,args:['--no-sandbox']});
const page=await browser.newPage({viewport:{width:1440,height:1100}});
const errors=[];page.on('pageerror',error=>errors.push(error.message));
const url=process.env.PREVIEW_URL || 'http://127.0.0.1:8767';
try {
 await page.goto(url);await page.getByRole('heading',{name:'Good morning, Rob.'}).waitFor();
 await page.screenshot({path:output+'/desktop.png',fullPage:true});
 await page.getByRole('button',{name:'Edit alarm ↗',exact:true}).click();
 await page.getByRole('dialog').waitFor();await page.screenshot({path:output+'/editor.png'});
 await page.keyboard.press('Escape');assert.equal(await page.locator('dialog').count(),1);
 assert.equal(await page.getByRole('dialog').isVisible(),false);
 await page.getByText('Prototype controls · explore offline and conflict behavior',{exact:true}).click();
 await page.getByRole('button',{name:'Take demo device offline'}).click();
 await page.getByRole('button',{name:'Edit alarm ↗',exact:true}).click();
 await page.getByLabel('Wake-up time',{exact:true}).fill('08:15');
 await page.getByRole('button',{name:'Send to demo device'}).click();
 await page.getByText('Waiting for the bedside clock',{exact:true}).waitFor();
 assert.equal(await page.getByRole('button',{name:'Simulate device acknowledgment'}).isDisabled(),true);
 assert.ok((await page.locator('.device-alarm').innerText()).includes('7:00 AM'));
 await page.locator('summary').click();await page.getByRole('button',{name:'Bring demo device online'}).click();
 await page.getByRole('button',{name:'Simulate device acknowledgment'}).click();
 assert.ok((await page.locator('.device-alarm').innerText()).includes('8:15 AM'));
 await page.getByRole('button',{name:'Edit alarm ↗',exact:true}).click();
 await page.getByLabel('Wake-up time',{exact:true}).fill('09:00');
 await page.getByRole('button',{name:'Send to demo device'}).click();
 await page.locator('summary').click();await page.getByRole('button',{name:'Simulate local edit to 6:45 AM'}).click();
 await page.getByRole('button',{name:'Simulate device acknowledgment'}).click();
 await page.getByText('The clock changed while you were editing.',{exact:true}).waitFor();
 assert.ok((await page.locator('.device-alarm').innerText()).includes('6:45 AM'));
 await page.screenshot({path:output+'/conflict.png',fullPage:true});
 await page.getByRole('button',{name:'Discard pending change'}).click();
 await page.getByLabel('Demo profile').selectOption('alex');
 assert.ok((await page.locator('.device-alarm').innerText()).includes('7:30 AM'));
 await page.goto(url+'/#privacy');
 assert.equal(await page.locator('.privacy-value').count(),0);
 await page.locator('#share-health').check();assert.equal(await page.locator('.privacy-value').count(),1);
 await page.getByLabel('Demo profile').selectOption('rob');assert.equal(await page.locator('.privacy-value').count(),0);
 for(const route of ['today','devices','alarms','music','assistant','connections','privacy']){
  await page.goto(url+'/#'+route);await page.locator('h1').waitFor();
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth<=window.innerWidth),true,'desktop overflow '+route);
 }
 await page.setViewportSize({width:390,height:844});
 await page.goto(url+'/#today');await page.getByRole('button',{name:'Reset demo'}).click();
 await page.screenshot({path:output+'/mobile.png',fullPage:true});
 for(const route of ['today','devices','alarms','music','assistant','connections','privacy']){
  await page.goto(url+'/#'+route);await page.locator('h1').waitFor();
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth<=window.innerWidth),true,'mobile overflow '+route);
 }
 await page.goto(url+'/#alarms');await page.getByRole('button',{name:'Edit alarm ↗',exact:true}).click();
 await page.screenshot({path:output+'/mobile-editor.png'});
 assert.equal(await page.evaluate(()=>document.querySelector('dialog').getBoundingClientRect().right<=innerWidth),true);
 assert.deepEqual(errors,[]);
 console.log('PASS desktop/mobile routes; dialog; offline pending/applied/conflict; profile isolation; private demo health; no browser exceptions');
} finally {await browser.close();}
