local pb = require "extern/lib/lib_pb"
function get_type_times(cards_type,max_value)
	if cards_type >= pb.get_ev("OX_CARD_TYPE", "OX_CARD_TYPE_OX_TWO") then
		return 10
	elseif cards_type == pb.get_ev("OX_CARD_TYPE","OX_CARD_TYPE_OX_ONE") then
		return max_value
	end
	return 1
end
function include_king(card)
	local bomb_num = 0
	for i=1,5 do
		if card[i] == 52 or card[i] == 53 then
			bomb_num = bomb_num + 1
		end
	end
	return bomb_num
end
function get_value(card) return math.floor(card / 4) end
function get_value_ox(val)
	if val >= 9 then return 10 end
	return val + 1
end
function get_color(card) return card % 4 end
function get_cards_type(cards)
	local king_num = include_king(cards)
	local list = {}
	for i=1,5 do
		list[i] = cards[i]
	end
	table.sort(list, function (a, b)
		return a > b
	end)
	local king_ox = 0
	local is_ten = false
	local repeat_times =0
	local last_value = nil
	local val_list = {}
	local four_same = false
	local same_value = nil
	local sum_value =0
	local bomb_num = 0
	for i =1,5 do
		local  val = math.floor(list[i]/4)
		if list[i] ~= 52 and list[i] ~= 53 then
			sum_value = sum_value + val +1
		end
		val_list[i] = val
		if val == 9 then
			is_ten = true
		elseif val > 9 and val < 13 then
			king_ox = king_ox + 1
		elseif val == 13 then
			bomb_num = bomb_num + 1
		end
	
		if list[i] ~= 52 and list[i] ~= 53 then
			if not last_value then
				last_value = val
				repeat_times = 1
			elseif last_value ~= val then
				if repeat_times ==4 then
					four_same = true
					same_value = list[i]
				end
				last_value = val
				if king_num == 0  then
					repeat_times = 1
				elseif king_num == 1 and repeat_times == 2 then
					repeat_times = 1
				elseif king_num == 2 and repeat_times < 2 then
					repeat_times = 1
				end
			else
				repeat_times = repeat_times +1
				same_value = list[i]
			end
		end
	end
	
	if sum_value <= 10 - king_num then
		return pb.get_ev("OX_CARD_TYPE","OX_CARD_TYPE_FIVE_SAMLL"),val_list,get_color(list[1])
	end
	if repeat_times == 4 - king_num or four_same then
		return pb.get_ev("OX_CARD_TYPE", "OX_CARD_TYPE_FOUR_SAMES"),same_value,get_color(list[1])
	end
	if king_ox == 5 - king_num then
		return pb.get_ev("OX_CARD_TYPE", "OX_CARD_TYPE_FIVE_KING"),val_list,get_color(list[1])
	end
	if king_ox == 4 - king_num and is_ten then
		return pb.get_ev("OX_CARD_TYPE", "OX_CARD_TYPE_FOUR_KING"),val_list,get_color(list[1])
	end
	local val_ox = {}
	for i=1,5 do
		val_ox[i] = get_value_ox(val_list[i])
	end
	if king_num == 2 then 
		return pb.get_ev("OX_CARD_TYPE", "OX_CARD_TYPE_OX_TWO"),val_list,get_color(list[1])
	elseif king_num == 1 then
		for i=2, 3 do
			for j=i+1, 4 do
				for k =j+1, 5 do
					if (val_ox[i] + val_ox[j] + val_ox[k]) %10 ==0 then
						return pb.get_ev("OX_CARD_TYPE", "OX_CARD_TYPE_OX_TWO"),val_list,get_color(list[1])
					end
				end
			end
		end             
	
		local max_value = 0
		for i=2, 4 do
			for j=i+1,5 do
				if (val_ox[i] + val_ox[j]) %10 == 0 then
					return pb.get_ev("OX_CARD_TYPE", "OX_CARD_TYPE_OX_TWO"),val_list,get_color(list[1])
				end
				if (val_ox[i] + val_ox[j]) %10 > max_value then
					max_value = (val_ox[i] + val_ox[j]) %10
				end
			end
		end
		return pb.get_ev("OX_CARD_TYPE","OX_CARD_TYPE_OX_ONE"),val_list,get_color(list[1]),max_value
	else  
		local is_three_eq_ten =false 
		local is_ox_two = false 
		local ox_num = 0
		for i=1,3 do
			for j =i+1,4 do
				for k=j+1,5 do
					if (val_ox[i] + val_ox[j] + val_ox[k]) %10 ==0 then
						is_three_eq_ten = true
						local other_sum =0
						for m=1,5 do
							if m ~=i and m ~=j and m~=k then
								other_sum = other_sum + val_ox[m]
							end
						end
						if(other_sum)%10 ==0 then
							is_ox_two = true
						else
							ox_num = other_sum %10
						end
					end
				end
			end
		end
		if is_ox_two then
			return pb.get_ev("OX_CARD_TYPE", "OX_CARD_TYPE_OX_TWO"),val_list,get_color(list[1])
		end
		if is_three_eq_ten then
			return pb.get_ev("OX_CARD_TYPE","OX_CARD_TYPE_OX_ONE"),val_list,get_color(list[1]),ox_num
		end
		return pb.get_ev("OX_CARD_TYPE","OX_CARD_TYPE_OX_NONE"), val_list, get_color(list[1])
	end
end
function compare_cards(first, second)
	if first.ox_type ~= second.ox_type then
		return first.ox_type > second.ox_type
	end
	if first.ox_type == pb.get_ev("OX_CARD_TYPE","OX_CARD_TYPE_OX_ONE") then
		if first.cards_times ~= second.cards_times then
			return first.cards_times > second.cards_times
		end
	end
	
	if first.ox_type == pb.get_ev("OX_CARD_TYPE", "OX_CARD_TYPE_FOUR_SAMES") then
		return first.val_list > second.val_list
	end
	for i=1,5 do
		local v1 = first.val_list[i]
		local v2 = second.val_list[i]
		if v1 > v2 then
			return true
		elseif v1 < v2 then
			return false
		else
			return first.color > second.color
		end
	end
	return first.color > second.color
end

function get_cards_odds(cards_times)
	local times = 1
	if cards_times < 7 then
		times = 1
	elseif cards_times >= 7 and cards_times < 10 then
		times = 2
	else
		times = 3
	end
	return times
end
