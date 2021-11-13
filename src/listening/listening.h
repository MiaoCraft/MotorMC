#pragma once
#include <pthread.h>

#include "listening.d.h"
#include "../world/entity/living/player/player.d.h"

#include "../main.h"
#include "../util/id_vector.h"
#include "../util/dll.h"
#include "../util/util.h"
#include "../util/lock_util.h"
#include "../io/packet/packet.h"
#include "../crypt/rsa.h"
#include "../crypt/cfb8.h"
#include "socket/socket.h"

#define LTG_UUID_UNPACK(uuid) (ltg_uuid_t) { uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7], uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15] }

struct ltg_listener {

	pthread_t thread;

	// address
	struct {
		int32_t socket;
		struct sockaddr_in addr;
		uint16_t port;
	} address;

	struct {
		pthread_mutex_t lock;
		utl_id_vector_t vector;
	} clients;
	
	struct {
		pthread_mutex_t lock;
		utl_dll_t list;
		atomic_uint_least32_t max;
	} online;

	cry_rsa_keypair_t keypair;

};

struct ltg_client {

	ltg_listener_t* listener;

	// client's thread
	pthread_t thread;
	
	// player entity (only non-null when in PLAY state)
	ent_player_t* entity;

	// compressors and decompressors
	struct {
		struct libdeflate_compressor* compressor;
		struct libdeflate_decompressor* decompressor;
	} compression;

	// textures (only non-null after auth)
	struct {
		string_t value;
		string_t signature;
	} textures;

	string_t username;

	// last recieved packet
	atomic_int_least64_t last_recv;

	atomic_int_least64_t ping;

	ltg_uuid_t uuid;

	struct {
		struct sockaddr_in addr;
		int size;
	} address;

	uint32_t online_node;

	uint32_t id;

	// address
	pthread_mutex_t lock;
	int32_t socket;

	ltg_locale_t locale;

	uint32_t keep_alive;

	struct {
		EVP_CIPHER_CTX* encrypt;
		EVP_CIPHER_CTX* decrypt;
		bool enabled : 1;
	} encryption;

	uint16_t protocol : 10;
	uint8_t render_distance : 6;
	enum {

		ltg_chat_enabled = 0,
		ltg_chat_commands_only = 1,
		ltg_chat_hidden = 2

	} chat_mode : 2;

	bool compression_enabled : 1;

	ltg_client_state_t state : 2;

};

extern void ltg_init();
extern void* t_ltg_run(void*);
extern void ltg_accept(ltg_client_t*);
extern void* t_ltg_client(void*);

static inline ltg_client_t* ltg_get_client_by_id(ltg_listener_t* listener, uint32_t id) {
	
	ltg_client_t* client = NULL;

	with_lock (&listener->clients.lock) {
		client = UTL_ID_VECTOR_GET_AS(ltg_client_t*, &listener->clients.vector, id);
	}

	return client;

}

extern bool ltg_handle_packet(ltg_client_t* client, pck_packet_t* packet);

extern void ltg_send(ltg_client_t*, pck_packet_t*);

extern void ltg_disconnect(ltg_client_t*);

extern void ltg_term(ltg_listener_t* listener);

static inline void ltg_uuid_to_string(ltg_uuid_t uuid, char* out) {
	
	out[0] = utl_hexmap[uuid[0] >> 4];
	out[1] = utl_hexmap[uuid[0] & 0xF];
	out[2] = utl_hexmap[uuid[1] >> 4];
	out[3] = utl_hexmap[uuid[1] & 0xF];
	out[4] = utl_hexmap[uuid[2] >> 4];
	out[5] = utl_hexmap[uuid[2] & 0xF];
	out[6] = utl_hexmap[uuid[3] >> 4];
	out[7] = utl_hexmap[uuid[3] & 0xF];
	out[8] = '-';
	out[9] = utl_hexmap[uuid[4] >> 4];
	out[10] = utl_hexmap[uuid[4] & 0xF];
	out[11] = utl_hexmap[uuid[5] >> 4];
	out[12] = utl_hexmap[uuid[5] & 0xF];
	out[13] = '-';
	out[14] = utl_hexmap[uuid[6] >> 4];
	out[15] = utl_hexmap[uuid[6] & 0xF];
	out[16] = utl_hexmap[uuid[7] >> 4];
	out[17] = utl_hexmap[uuid[7] & 0xF];
	out[18] = '-';
	out[19] = utl_hexmap[uuid[8] >> 4];
	out[20] = utl_hexmap[uuid[8] & 0xF];
	out[21] = utl_hexmap[uuid[9] >> 4];
	out[22] = utl_hexmap[uuid[9] & 0xF];
	out[23] = '-';
	out[24] = utl_hexmap[uuid[10] >> 4];
	out[25] = utl_hexmap[uuid[10] & 0xF];
	out[26] = utl_hexmap[uuid[11] >> 4];
	out[27] = utl_hexmap[uuid[11] & 0xF];
	out[28] = utl_hexmap[uuid[12] >> 4];
	out[29] = utl_hexmap[uuid[12] & 0xF];
	out[30] = utl_hexmap[uuid[13] >> 4];
	out[31] = utl_hexmap[uuid[13] & 0xF];
	out[32] = utl_hexmap[uuid[14] >> 4];
	out[33] = utl_hexmap[uuid[14] & 0xF];
	out[34] = utl_hexmap[uuid[15] >> 4];
	out[35] = utl_hexmap[uuid[15] & 0xF];
	out[36] = 0;

}

// API

static inline string_t ltg_client_get_username(const ltg_client_t* client) {
	return client->username;
}

static inline const byte_t* ltg_client_get_uuid(const ltg_client_t* client) {
	return client->uuid;
}

static inline uint16_t ltg_client_get_protocol(const ltg_client_t* client) {
	return client->protocol;
}

static inline uint8_t ltg_client_get_render_distance(const ltg_client_t* client) {
	return client->render_distance;
}

static inline int64_t ltg_client_get_ping(const ltg_client_t* client) {
	return client->ping;
}

static inline string_t ltg_client_get_textures(const ltg_client_t* client) {
	return client->textures.value;
}

static inline ltg_locale_t ltg_client_get_locale(const ltg_client_t* client) {
	return client->locale;
}

static inline ent_player_t* ltg_client_get_entity(const ltg_client_t* client) {
	return client->entity;
}

static inline pthread_t ltg_get_thread(const ltg_listener_t* listener) {
	return listener->thread;
}

static inline uint32_t ltg_get_client_count(ltg_listener_t* listener) {
	
	uint32_t size = 0;
	
	with_lock (&listener->clients.lock) {
		size = utl_id_vector_size(&listener->clients.vector);
	}

	return size;

}

static inline pthread_t ltg_client_get_thread(const ltg_client_t* client) {
	return client->thread;
}

static inline ltg_client_state_t ltg_client_get_state(const ltg_client_t* client) {
	return client->state;
}

static inline bool ltg_client_is_encryption_enabled(const ltg_client_t* client) {
	return client->encryption.enabled;
}

static inline uint32_t ltg_get_online_max(const ltg_listener_t* listener) {
	return listener->online.max;
}

static inline void ltg_add_online(ltg_listener_t* listener, ltg_client_t* client) {
	
	with_lock (&listener->online.lock) {
		client->online_node = utl_dll_push(&listener->online.list, client);
	}

}

static inline uint32_t ltg_get_online_count(ltg_listener_t* listener) {

	uint32_t count = 0;

	with_lock (&listener->online.lock) {
		count = utl_dll_length(&listener->online.list);
	}

	return count;
	
}

static inline utl_dll_iterator_t ltg_get_player_iterator(ltg_listener_t* listener) {

	return utl_dll_get_iterator(&listener->online.list);
	
}

static inline ltg_client_t* ltg_player_iterator_next(ltg_listener_t* listener, utl_dll_iterator_t* iterator) {

	ltg_client_t* client = NULL;

	with_lock (&listener->online.lock) {
		client = utl_dll_iterator_next(iterator);
	}

	return client;

}

static inline cry_rsa_keypair_t* ltg_get_rsa_keys(ltg_listener_t* listener) {
	return &listener->keypair;
}