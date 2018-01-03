package constantin.fpv_vr;

import java.util.Arrays;

public class Distortion
{
    private static final float[] CARDBOARD_V2_2_COEFFICIENTS = { 0.34F, 0.55F };
    private static final float[] CARDBOARD_V1_COEFFICIENTS = { 0.441F, 0.156F };
    private float[] coefficients;

    public static Distortion cardboardV1Distortion()
    {
        Distortion params = new Distortion();
        params.coefficients = ((float[])CARDBOARD_V1_COEFFICIENTS.clone());

        return params;
    }

    public Distortion()
    {
        this.coefficients = ((float[])CARDBOARD_V2_2_COEFFICIENTS.clone());
    }

    public Distortion(Distortion other)
    {
        setCoefficients(other.coefficients);
    }

    public static Distortion parseFromProtobuf(float[] coefficients)
    {
        Distortion distortion = new Distortion();
        distortion.setCoefficients(coefficients);
        return distortion;
    }

    public float[] toProtobuf()
    {
        return (float[])this.coefficients.clone();
    }

    public void setCoefficients(float[] coefficients)
    {
        this.coefficients = (coefficients != null ? (float[])coefficients.clone() : new float[0]);
    }

    public float[] getCoefficients()
    {
        return this.coefficients;
    }

    public float distortionFactor(float radius)
    {
        float result = 1.0F;
        float rFactor = 1.0F;
        float rSquared = radius * radius;
        for (float ki : this.coefficients)
        {
            rFactor *= rSquared;
            result += ki * rFactor;
        }
        return result;
    }

    public float distort(float radius)
    {
        return radius * distortionFactor(radius);
    }

    public float distortInverse(float radius)
    {
        float r0 = radius / 0.9F;
        float r1 = radius * 0.9F;
        float dr0 = radius - distort(r0);
        while (Math.abs(r1 - r0) > 1.0E-4D)
        {
            float dr1 = radius - distort(r1);
            float r2 = r1 - dr1 * ((r1 - r0) / (dr1 - dr0));
            r0 = r1;
            r1 = r2;
            dr0 = dr1;
        }
        return r1;
    }

    private static double[] solveLinear(double[][] a, double[] y)
    {
        int n = a[0].length;
        for (int j = 0; j < n - 1; j++) {
            for (int k = j + 1; k < n; k++)
            {
                double p = a[k][j] / a[j][j];
                for (int i = j + 1; i < n; i++) {
                    a[k][i] -= p * a[j][i];
                }
                y[k] -= p * y[j];
            }
        }
        double[] x = new double[n];
        for (int j = n - 1; j >= 0; j--)
        {
            double v = y[j];
            for (int i = j + 1; i < n; i++) {
                v -= a[j][i] * x[i];
            }
            x[j] = (v / a[j][j]);
        }
        return x;
    }

    private static double[] solveLeastSquares(double[][] matA, double[] vecY)
    {
        int numSamples = matA.length;
        int numCoefficients = matA[0].length;

        double[][] matATA = new double[numCoefficients][numCoefficients];
        for (int k = 0; k < numCoefficients; k++) {
            for (int j = 0; j < numCoefficients; j++)
            {
                double sum = 0.0D;
                for (int i = 0; i < numSamples; i++) {
                    sum += matA[i][j] * matA[i][k];
                }
                matATA[j][k] = sum;
            }
        }
        double[] vecATY = new double[numCoefficients];
        for (int j = 0; j < numCoefficients; j++)
        {
            double sum = 0.0D;
            for (int i = 0; i < numSamples; i++) {
                sum += matA[i][j] * vecY[i];
            }
            vecATY[j] = sum;
        }
        return solveLinear(matATA, vecATY);
    }

    public Distortion getApproximateInverseDistortion(float maxRadius, int numCoefficients)
    {
        int numSamples = 100;

        double[][] matA = new double[100][numCoefficients];
        double[] vecY = new double[100];
        for (int i = 0; i < 100; i++)
        {
            float r = maxRadius * (i + 1) / 100.0F;
            double rp = distort(r);
            double v = rp;
            for (int j = 0; j < numCoefficients; j++)
            {
                v *= rp * rp;
                matA[i][j] = v;
            }
            vecY[i] = (r - rp);
        }
        double[] vecK = solveLeastSquares(matA, vecY);

        float[] coefficients = new float[vecK.length];
        for (int i = 0; i < vecK.length; i++) {
            coefficients[i] = ((float)vecK[i]);
        }
        Distortion inverse = new Distortion();
        inverse.setCoefficients(coefficients);
        return inverse;
    }

    @Deprecated
    public Distortion getApproximateInverseDistortion(float maxRadius)
    {
        return getApproximateInverseDistortion(maxRadius, 2);
    }

    public boolean equals(Object other)
    {
        if (other == null) {
            return false;
        }
        if (other == this) {
            return true;
        }
        if (!(other instanceof Distortion)) {
            return false;
        }
        Distortion o = (Distortion)other;
        return Arrays.equals(this.coefficients, o.coefficients);
    }

    public String toString()
    {
        StringBuilder builder = new StringBuilder().append("{\n").append("  coefficients: [");
        for (int i = 0; i < this.coefficients.length; i++)
        {
            builder.append(Float.toString(this.coefficients[i]));
            if (i < this.coefficients.length - 1) {
                builder.append(", ");
            }
        }
        builder.append("],\n}");
        return builder.toString();
    }
}