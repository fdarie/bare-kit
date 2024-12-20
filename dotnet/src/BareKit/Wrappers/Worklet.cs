using System;
using System.Runtime.InteropServices;

namespace BareKit
{
    public static class Worklet
    {
        // Define the library name based on platform
        private const string LibraryName =
#if WINDOWS
            "mylib.dll";
#elif LINUX
            "libmylib.so";
#elif MACOS
            "libmylib.dylib";
#else
            "mylib"; // Fallback
#endif

        // Import functions from the native library
        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr bare_worklet_init_wrapper(int memory_limit, [MarshalAs(UnmanagedType.LPStr)] string assets);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bare_worklet_start_wrapper(IntPtr worklet, [MarshalAs(UnmanagedType.LPStr)] string filename, IntPtr source, int argc, IntPtr argv);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bare_worklet_suspend_wrapper(IntPtr worklet, int linger);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bare_worklet_resume_wrapper(IntPtr worklet);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bare_worklet_terminate_wrapper(IntPtr worklet);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr bare_worklet_endpoint_wrapper(IntPtr worklet);

        // Delegate matching the callback signature
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void BareWorkletPushCallback(IntPtr context, string error, IntPtr reply);

        [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bare_worklet_push_wrapper(IntPtr worklet, ref UvBuf payload, BareWorkletPushCallback callback, IntPtr context);

        // Define uv_buf_t structure
        [StructLayout(LayoutKind.Sequential)]
        public struct UvBuf
        {
            public IntPtr basePtr;
            public UIntPtr len;
        }

        // Example usage of the callback
        public static void ExamplePushCallback(IntPtr context, string error, IntPtr reply)
        {
            if (!string.IsNullOrEmpty(error))
            {
                Console.WriteLine($"Error: {error}");
                return;
            }

            if (reply != IntPtr.Zero)
            {
                // Assuming reply is a byte buffer
                int length = (int)Marshal.ReadIntPtr(reply, sizeof(int)); // Adjust based on actual structure
                byte[] buffer = new byte[length];
                Marshal.Copy(reply, buffer, 0, length);
                Console.WriteLine($"Received reply: {BitConverter.ToString(buffer)}");
            }
        }

        // Managed method to push data
        public static int PushData(IntPtr worklet, byte[] data)
        {
            if (worklet == IntPtr.Zero || data == null)
                throw new ArgumentException("Invalid arguments");

            // Allocate unmanaged memory for the payload
            IntPtr payloadPtr = Marshal.AllocHGlobal(data.Length);
            Marshal.Copy(data, 0, payloadPtr, data.Length);

            UvBuf payload = new UvBuf
            {
                basePtr = payloadPtr,
                len = new UIntPtr((uint)data.Length)
            };

            // Create a GCHandle to pass managed data to unmanaged code if needed
            // For simplicity, using IntPtr.Zero for context
            BareWorkletPushCallback callback = ExamplePushCallback;

            int result = bare_worklet_push_wrapper(worklet, ref payload, callback, IntPtr.Zero);

            // Note: Depending on the native implementation, you might need to keep the delegate alive
            // to prevent it from being garbage collected before the callback is invoked.

            return result;
        }
    }
}
