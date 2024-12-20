// IPC.c

#include "../../../../shared/ipc.h"
#include "../../../../shared/worklet.h"
#include "uv.h" // libuv header
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Define calling convention for cross-platform compatibility
#if defined(_WIN32) || defined(_WIN64)
#define API_CALL __declspec(dllexport)
#else
#define API_CALL
#endif

// Callback type definition for poll operations
typedef int (*bare_ipc_poll_callback)(int fd, int events, void *context);

// Redefine the context structure
typedef struct {
  bare_ipc_t handle;
  uv_loop_t *loop;
  uv_poll_t poll_handle;
  bare_ipc_poll_callback callback;
  void *callback_context;
  int fd; // Ensure this is initialized correctly
} bare_ipc_context_t;

typedef struct {
  bare_ipc_msg_t handle;
} bare_ipc_msg_context_t;

// Function to initialize the IPC
API_CALL bare_ipc_context_t *
bare_ipc_init_wrapper(const char *endpoint) {
  int err;

  bare_ipc_context_t *context = malloc(sizeof(bare_ipc_context_t));
  if (!context) {
    return NULL; // Allocation failed
  }

  memset(context, 0, sizeof(bare_ipc_context_t));

  err = bare_ipc_init(&context->handle, endpoint);
  if (err != 0) {
    free(context);
    return NULL; // Initialization failed
  }

  context->fd = bare_ipc_fd(&context->handle);
  context->loop = uv_default_loop(); // Use the default libuv loop

  context->poll_handle.data = context; // Associate context with the poll handle

  context->callback = NULL;
  context->callback_context = NULL;

  return context;
}

// Function to destroy the IPC
API_CALL int
bare_ipc_destroy_wrapper(bare_ipc_context_t *context) {
  if (!context) {
    return -1; // Invalid argument
  }

  bare_ipc_destroy(&context->handle);

  // Stop polling if it's active
  if (!uv_is_closing((uv_handle_t *) &context->poll_handle)) {
    uv_poll_stop(&context->poll_handle);
  }

  // Close the poll handle
  uv_close((uv_handle_t *) &context->poll_handle, NULL);

  free(context);
  return 0; // Success
}

// Function to create a new IPC message
API_CALL bare_ipc_msg_context_t *
bare_ipc_message_wrapper() {
  bare_ipc_msg_context_t *msg_context = malloc(sizeof(bare_ipc_msg_context_t));
  if (!msg_context) {
    return NULL; // Allocation failed
  }

  memset(msg_context, 0, sizeof(bare_ipc_msg_context_t));

  return msg_context;
}

// Function to read from IPC
API_CALL int
bare_ipc_read_wrapper(bare_ipc_context_t *context, bare_ipc_msg_context_t *msg_context, void **data, size_t *len) {
  if (!context || !msg_context || !data || !len) {
    return -1; // Invalid arguments
  }

  int err = bare_ipc_read(&context->handle, &msg_context->handle, data, len);
  if (err != 0 && err != bare_ipc_would_block) {
    return err; // Read failed
  }

  return err; // Success or would block
}

// Function to write to IPC
API_CALL int
bare_ipc_write_wrapper(bare_ipc_context_t *context, bare_ipc_msg_context_t *msg_context, const void *source, int len) {
  if (!context || !msg_context || !source || len <= 0) {
    return -1; // Invalid arguments
  }

  int err = bare_ipc_write(&context->handle, &msg_context->handle, source, len);
  if (err != 0 && err != bare_ipc_would_block) {
    return err; // Write failed
  }

  return err; // Success or would block
}

// Function to release IPC message
API_CALL void
bare_ipc_release_wrapper(bare_ipc_msg_context_t *msg_context) {
  if (!msg_context) {
    return;
  }

  bare_ipc_release(&msg_context->handle);
  free(msg_context);
}

// Internal callback function for poll operations
static void
bare_ipc_poll_cb(uv_poll_t *handle, int status, int events) {
  bare_ipc_context_t *context = (bare_ipc_context_t *) handle->data;

  if (context->callback) {
    // Use context->fd instead of handle->fd
    int result = context->callback(context->fd, events, context->callback_context);
    // Handle the result if necessary
  }
}

// Function to set up polling with a callback
API_CALL int
bare_ipc_poll_wrapper(bare_ipc_context_t *context, bare_ipc_poll_callback callback, void *callback_context, int events) {
  if (!context) {
    return -1; // Invalid argument
  }

  context->callback = callback;
  context->callback_context = callback_context;

  if (events) {
    int err = uv_poll_init(context->loop, &context->poll_handle, context->fd);
    if (err < 0) {
      return err; // Initialization failed
    }

    err = uv_poll_start(&context->poll_handle, events, bare_ipc_poll_cb);
    if (err < 0) {
      return err; // Start polling failed
    }
  } else {
    // Stop polling if events is 0
    uv_poll_stop(&context->poll_handle);
  }

  return 0; // Success
}
