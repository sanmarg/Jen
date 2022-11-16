void pointer_contents_change( const int* i )
{
    *i = 1;
}


int main()
{
    int i = 0;
    pointer_contents_change( &i );
    std::cout << "i = " << i << "\n";
    return 0;
}