

#include <em_common.h>
#include "sl_btmesh.h"
#include "sl_btmesh_generic_base.h"
#include "sl_btmesh_event_log.h"
#include "sl_btmesh_provisioning_decorator.h"
#include "sl_btmesh_generic_base.h"
#include "sl_btmesh_lighting_server.h"
#include "sl_btmesh_ctl_server.h"
#include "sl_btmesh_lc_server.h"
#include "sl_btmesh_factory_reset.h"
#include "sl_btmesh_friend.h"
#include "sl_btmesh_scene_server.h"
#include "sl_btmesh_scheduler_server.h"
#include "sl_btmesh_time_server.h"

/** @brief Table of used BGAPI classes */
static const struct sli_bgapi_class * const btmesh_class_table[] =
{
  SL_BTMESH_BGAPI_CLASS(friend),
  SL_BTMESH_BGAPI_CLASS(generic_server),
  SL_BTMESH_BGAPI_CLASS(health_server),
  SL_BTMESH_BGAPI_CLASS(lc_server),
  SL_BTMESH_BGAPI_CLASS(lc_setup_server),
  SL_BTMESH_BGAPI_CLASS(proxy),
  SL_BTMESH_BGAPI_CLASS(proxy_server),
  SL_BTMESH_BGAPI_CLASS(scene_server),
  SL_BTMESH_BGAPI_CLASS(scene_setup_server),
  SL_BTMESH_BGAPI_CLASS(scheduler_server),
  SL_BTMESH_BGAPI_CLASS(time_server),
  SL_BTMESH_BGAPI_CLASS(node),
  NULL
};

void sl_btmesh_init(void)
{
  sl_btmesh_init_classes(btmesh_class_table);
  sl_btmesh_generic_base_init();
}

SL_WEAK void sl_btmesh_on_event(sl_btmesh_msg_t* evt)
{
  (void)(evt);
}

void sl_btmesh_process_event(sl_btmesh_msg_t *evt)
{
  sl_btmesh_handle_btmesh_logging_events(evt);
  sl_btmesh_handle_provisioning_decorator_event(evt);
  sl_btmesh_generic_base_on_event(evt);
  sl_btmesh_lighting_server_on_event(evt);
  sl_btmesh_ctl_server_on_event(evt);
  sl_btmesh_lc_server_on_event(evt);
  sl_btmesh_factory_reset_on_event(evt);
  sl_btmesh_friend_on_event(evt);
  sl_btmesh_scene_server_on_event(evt);
  sl_btmesh_scheduler_server_on_event(evt);
  sl_btmesh_time_server_on_event(evt);
  sl_btmesh_on_event(evt);
}

SL_WEAK bool sl_btmesh_can_process_event(uint32_t len)
{
  (void)(len);
  return true;
}

void sl_btmesh_step(void)
{
  sl_btmesh_msg_t evt;

  uint32_t event_len = sl_btmesh_event_pending_len();
  // For preventing from data loss, the event will be kept in the stack's queue
  // if application cannot process it at the moment.
  if ((event_len == 0) || (!sl_btmesh_can_process_event(event_len))) {
    return;
  }

  // Pop (non-blocking) a Bluetooth stack event from event queue.
  sl_status_t status = sl_btmesh_pop_event(&evt);
  if(status != SL_STATUS_OK){
    return;
  }
  sl_btmesh_process_event(&evt);
}
