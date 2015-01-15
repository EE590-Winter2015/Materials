using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CSharpComponent
{
    public sealed class CSharpObject
    {
        // Declare our private data member
        private int counter;

        // Create the default constructor
        public CSharpObject()
        {
            Debug.WriteLine("[C# ] Creating CSharpObject");
            this.counter = 0;
        }

        // Create the constructor that takes in an explicit initial value
        public CSharpObject(int initialValue)
        {
            Debug.WriteLine("[C# ] Creating CSharpObject with intial value {0}", initialValue);
            this.counter = initialValue;
        }

        // Create the destructor
        ~CSharpObject()
        {
            Debug.WriteLine("[C# ] Destroying CSharpObject with counter {0}", this.counter);
        }

        public int increment(int delta)
        {
            this.counter += delta;
            return this.counter;
        }
    }
}
