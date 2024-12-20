// Worklet.c

#include "../../../../shared/worklet.h"
#include "uv.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// Define calling convention for cross-platform compatibility
#if defined(_WIN32) || defined(_WIN64)
#define API_CALL __declspec(dllexport)
#else
#define API_CALL
#endif

// Callback type definition for push operations
typedef void (*bare_worklet_push_callback)(void *context, const char *error, const uv_buf_t *reply);

// Redefine the context structure
typedef struct {
  bare_worklet_push_callback callback; // .NET callback function
  void *context;                       // User-defined context
} bare_worklet_push_context_t;

// Function to initialize the worklet
API_CALL bare_worklet_t *
bare_worklet_init_wrapper(int memory_limit, const char *assets) {
  int err;

  bare_worklet_t *worklet = malloc(sizeof(bare_worklet_t));
  if (!worklet) {
    return NULL; // Allocation failed
  }

  bare_worklet_options_t options;
  options.memory_limit = memory_limit;

  if (assets) {
    options.assets = strdup(assets); // Duplicate the string to manage memory
    if (!options.assets) {
      free(worklet);
      return NULL; // Allocation failed
    }
  } else {
    options.assets = NULL;
  }

  err = bare_worklet_init(worklet, &options);
  if (err != 0) {
    if (options.assets) {
      free((void *) options.assets);
    }
    free(worklet);
    return NULL; // Initialization failed
  }

  return worklet;
}

// Function to start the worklet
API_CALL int
bare_worklet_start_wrapper(bare_worklet_t *worklet, const char *filename, const uv_buf_t *source, int argc, const char **argv) {
  if (!worklet || !filename) {
    return -1; // Invalid arguments
  }

  int err = bare_worklet_start(worklet, filename, source, argc, argv);
  return err;
}

// Function to suspend the worklet
API_CALL int
bare_worklet_suspend_wrapper(bare_worklet_t *worklet, int linger) {
  if (!worklet) {
    return -1; // Invalid argument
  }

  int err = bare_worklet_suspend(worklet, linger);
  return err;
}

// Function to resume the worklet
API_CALL int
bare_worklet_resume_wrapper(bare_worklet_t *worklet) {
  if (!worklet) {
    return -1; // Invalid argument
  }

  int err = bare_worklet_resume(worklet);
  return err;
}

// Function to terminate the worklet
API_CALL int
bare_worklet_terminate_wrapper(bare_worklet_t *worklet) {
  if (!worklet) {
    return -1; // Invalid argument
  }

  int err = bare_worklet_terminate(worklet);
  if (err != 0) {
    return err; // Termination failed
  }

  bare_worklet_destroy(worklet);
  if (worklet->data) {
    free(worklet->data); // Assuming worklet->data was dynamically allocated
  }
  free(worklet); // Free the worklet handle

  return 0; // Success
}

// Function to get the endpoint of the worklet
API_CALL const char *
bare_worklet_endpoint_wrapper(bare_worklet_t *worklet) {
  if (!worklet) {
    return NULL; // Invalid argument
  }

  return (const char *) worklet->endpoint;
}

// Internal callback function for push operations
static void
bare_worklet_on_push(bare_worklet_push_t *req, const char *error, const uv_buf_t *reply) {
  bare_worklet_push_context_t *context = (bare_worklet_push_context_t *) req->data;

  if (context && context->callback) {
    // Invoke the callback
    context->callback(context->context, error, reply);
  }

  // Clean up
  if (context) {
    free(context);
  }

  free(req);
}

// Function to push data to the worklet with a callback
API_CALL int
bare_worklet_push_wrapper(bare_worklet_t *worklet, const uv_buf_t *payload, bare_worklet_push_callback callback, void *context) {
  if (!worklet || !payload || !callback) {
    return -1; // Invalid arguments
  }

  bare_worklet_push_t *push_req = malloc(sizeof(bare_worklet_push_t));
  if (!push_req) {
    return -1; // Allocation failed
  }

  bare_worklet_push_context_t *push_context = malloc(sizeof(bare_worklet_push_context_t));
  if (!push_context) {
    free(push_req);
    return -1; // Allocation failed
  }

  push_context->callback = callback;
  push_context->context = context;

  push_req->data = (void *) push_context;

  int err = bare_worklet_push(worklet, push_req, payload, bare_worklet_on_push);
  if (err != 0) {
    free(push_context);
    free(push_req);
    return err; // Push failed
  }

  return 0; // Success
}
