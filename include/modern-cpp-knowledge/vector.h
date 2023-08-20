namespace vector
{
  class Vector
  {
   public:
    explicit Vector(unsigned long size);
    double& operator[](int idx);
    unsigned long size();

    ~Vector()
    {
      delete[] elem;
    }

   private:
    double* elem;
    unsigned long sz;
  };
}  // namespace vector