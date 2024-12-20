using System;
using System.Runtime.InteropServices;
using System.Text;

namespace BareKit
{
    public static class Ipc
    {
        // Define the library name based on platform
#if WINDOWS
        private const string LibraryName = "mylib.dll";
#elif LINUX
        private const string LibraryName = "libmylib.so";
#elif MACOS
        private const string LibraryName = "libmylib.dylib";
#else
        private const string LibraryName = "mylib"; // Fallback
#endif

        // Import functions from the native library
        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr bare_ipc_init_wrapper([MarshalAs(UnmanagedType.LPStr)] string endpoint);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bare_ipc_destroy_wrapper(IntPtr context);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr bare_ipc_message_wrapper();

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bare_ipc_read_wrapper(IntPtr context, IntPtr msg_context, out IntPtr data, out UIntPtr len);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bare_ipc_write_wrapper(IntPtr context, IntPtr msg_context, IntPtr source, int len);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bare_ipc_release_wrapper(IntPtr msg_context);

        // Delegate matching the callback signature
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int BareIpcPollCallback(int fd, int events, IntPtr context);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bare_ipc_poll_wrapper(IntPtr context, BareIpcPollCallback callback, Int32 events);

        // Define any additional structures if necessary
        [StructLayout(LayoutKind.Sequential)]
        public struct UvBuf
        {
            public IntPtr basePtr;
            public UIntPtr len;
        }

        // Example usage of the callback
        private static BareIpcPollCallback _pollCallbackDelegate = ExamplePollCallback;

        private static int ExamplePollCallback(int fd, int events, IntPtr context)
        {
            // Handle poll events here
            Console.WriteLine($"Poll Callback: fd={fd}, events={events}");
            return 0; // Return appropriate value based on callback logic
        }

        // Managed method to initialize IPC
        public static IntPtr InitializeIPC(string endpoint)
        {
            IntPtr ipcContext = bare_ipc_init_wrapper(endpoint);
            if (ipcContext == IntPtr.Zero)
            {
                throw new InvalidOperationException("Failed to initialize IPC.");
            }
            return ipcContext;
        }

        // Managed method to destroy IPC
        public static void DestroyIPC(IntPtr ipcContext)
        {
            int result = bare_ipc_destroy_wrapper(ipcContext);
            if (result != 0)
            {
                throw new InvalidOperationException($"Failed to destroy IPC. Error code: {result}");
            }
        }

        // Managed method to create a new IPC message
        public static IntPtr CreateIPCMessage()
        {
            IntPtr msgContext = bare_ipc_message_wrapper();
            if (msgContext == IntPtr.Zero)
            {
                throw new InvalidOperationException("Failed to create IPC message.");
            }
            return msgContext;
        }

        // Managed method to read from IPC
        public static byte[] ReadIPC(IntPtr ipcContext, IntPtr msgContext)
        {
            IntPtr data;
            UIntPtr len;
            int result = bare_ipc_read_wrapper(ipcContext, msgContext, out data, out len);
            if (result != 0 && result != bare_ipc_would_block)
            {
                throw new InvalidOperationException($"IPC read failed. Error code: {result}");
            }

            if (result == bare_ipc_would_block)
            {
                return null; // No data available
            }

            byte[] buffer = new byte[len.ToUInt32()];
            Marshal.Copy(data, buffer, 0, buffer.Length);
            return buffer;
        }

        // Managed method to write to IPC
        public static bool WriteIPC(IntPtr ipcContext, IntPtr msgContext, byte[] data)
        {
            if (data == null || data.Length == 0)
                throw new ArgumentException("Data cannot be null or empty.", nameof(data));

            IntPtr dataPtr = Marshal.AllocHGlobal(data.Length);
            try
            {
                Marshal.Copy(data, 0, dataPtr, data.Length);
                int result = bare_ipc_write_wrapper(ipcContext, msgContext, dataPtr, data.Length);
                if (result != 0 && result != bare_ipc_would_block)
                {
                    throw new InvalidOperationException($"IPC write failed. Error code: {result}");
                }

                return result != bare_ipc_would_block;
            }
            finally
            {
                Marshal.FreeHGlobal(dataPtr);
            }
        }

        // Managed method to release IPC message
        public static void ReleaseIPCMessage(IntPtr msgContext)
        {
            bare_ipc_release_wrapper(msgContext);
        }

        // Managed method to set up polling
        public static void SetupPolling(IntPtr ipcContext)
        {
            int events = 0; //POLLIN | POLLOUT
            int result = bare_ipc_poll_wrapper(ipcContext, _pollCallbackDelegate, events);
            if (result != 0)
            {
                throw new InvalidOperationException($"Failed to set up polling. Error code: {result}");
            }
        }

        // Define constants as per your native library's definitions
        private const int bare_ipc_would_block = 0;
    }
}
