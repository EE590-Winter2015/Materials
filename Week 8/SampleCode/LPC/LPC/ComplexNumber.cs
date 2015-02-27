using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LPC
{
    class ComplexNumber
    {
        protected float real, imag;

        public ComplexNumber(float real, float imag)
        {
            this.real = real;
            this.imag = imag;
        }

        public static ComplexNumber operator +(ComplexNumber c1, ComplexNumber c2)
        {
            return new ComplexNumber(c1.real + c2.real, c1.imag + c2.imag);
        }

        public static ComplexNumber operator -(ComplexNumber c1, ComplexNumber c2)
        {
            return new ComplexNumber(c1.real - c2.real, c1.imag - c2.imag);
        }

        public static ComplexNumber operator *(ComplexNumber c1, ComplexNumber c2)
        {
            return new ComplexNumber(c1.real * c2.real - c1.imag * c2.imag, c1.real * c2.imag + c1.imag * c2.real);
        }

        public static ComplexNumber operator *(ComplexNumber c1, float k)
        {
            return new ComplexNumber(c1.real * k, c1.imag * k);
        }

        public static ComplexNumber operator *( float k, ComplexNumber c1 )
        {
            return c1 * k;
        }

        public float mag()
        {
            return (float)Math.Sqrt(real * real + imag * imag);
        }


    }
}
