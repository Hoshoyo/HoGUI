#include "event.h"

#define EVENT_QUEUE_LENGTH 65536
typedef struct {
	Event events[EVENT_QUEUE_LENGTH];
	int   base;
	int   end;
} Event_Queue;

static Event_Queue event_queue;

void event_push(Event* event) {
	event_queue.events[event_queue.end] = *event;
	event_queue.end = (event_queue.end + 1) % EVENT_QUEUE_LENGTH;
}

void event_queue_clear() {
	for (int i = 0; i < EVENT_QUEUE_LENGTH; ++i) {
		event_queue.events[i].type = EVENT_NULL;
	}
	event_queue.base = 0;
	event_queue.end = 0;
}

bool event_pop(Event* e) {
	int previous_base = event_queue.base;
	*e = event_queue.events[event_queue.base];
	
	if (e->type != EVENT_NULL) {
		event_queue.base = (event_queue.base + 1) % EVENT_QUEUE_LENGTH;
		event_queue.events[previous_base].type = EVENT_NULL;
		return true;
	}
	return false;
}

bool event_peek(Event* e, int index) {
	*e = event_queue.events[(event_queue.base + index) % EVENT_QUEUE_LENGTH];

	if(e->type != EVENT_NULL) {
		return true;
	}

	return false;
}