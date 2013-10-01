program main
  real, dimension(100,100,100) :: d
  real :: x,y
  integer :: i,j,k

  i = 20
  j = 30
  k = 73
  x = d(i,j,k)
  y = sqrt(d(i,i+1,k))
  x = d(j,k,i) + y
end program main
