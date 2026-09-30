#include "alarm_output.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    assert(alarm_output_local(1,0));assert(!alarm_output_session());alarm_output_local(0,1);
    alarm_output_enable(true);assert(!alarm_output_local(1,10));uint32_t first=alarm_output_session();assert(first);
    assert(!alarm_output_local(1,8009));assert(alarm_output_local(1,8010));
    alarm_output_confirm(first,8011);assert(alarm_output_local(1,8012)); /* late cannot cancel fallback */
    assert(!alarm_output_local(0,8013));assert(!alarm_output_session());
    assert(!alarm_output_local(1,9000));uint32_t second=alarm_output_session();assert(second!=first);
    alarm_output_confirm(first,9001);assert(!alarm_output_local(1,9002));
    alarm_output_confirm(second,9003);assert(!alarm_output_local(1,9004));
    alarm_output_confirm(second,11000);assert(!alarm_output_local(1,13000));
    assert(alarm_output_local(1,14000));alarm_output_confirm(second,14001);assert(alarm_output_local(1,14002));
    alarm_output_local(0,15000);alarm_output_local(1,16000);alarm_output_enable(false);
    assert(alarm_output_local(1,16001)&&!alarm_output_session());
    alarm_output_local(0,0);alarm_output_enable(true);
    assert(!alarm_output_local(1,UINT32_MAX-4000));assert(!alarm_output_local(1,3000));assert(alarm_output_local(1,4000));
    alarm_output_local(0,5000);alarm_output_local(1,6000);
    alarm_output_confirm(alarm_output_session(),15000);assert(alarm_output_local(1,15001));
    puts("PASS remote alarm local policy: disabled default, deadline, stale/wrong session, lost lease, latched fallback, cancellation and timer wrap");
}
