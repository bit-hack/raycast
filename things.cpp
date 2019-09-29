#include "map.h"
#include "things.h"
#include "pfields.h"
#include "spatial.h"


thing_t *thing_create_imp();
thing_t *thing_create_player();

void thing_manager_t::tick() {
  for (auto &g : things) {
    for (auto &t : g) {
      t->tick();
    }
  }
}

thing_t *thing_manager_t::create(thing_type_t type) {
  thing_t *t = nullptr;
  switch (type) {
  case IMP:    t = thing_create_imp(); break;
  case PLAYER: t = thing_create_player(); break;
  }
  if (t) {
    t->on_create();
    things[t->type].push_back(t);
  }
  return t;
}

void thing_t::move(const vec3f_t &p) {
  service.spatial->remove(this);
  pos = p;
  service.spatial->insert(this);
}
