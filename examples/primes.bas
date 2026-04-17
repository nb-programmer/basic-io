print(2)

i = 3

while true then
    p_sub = 2
    is_prime = true
    while p_sub < i then
        if (i % p_sub) = 0 then
            is_prime = false
        end
        p_sub = p_sub + 1
    end

    if is_prime then
        print(i)
    end

    i = i + 2
end
