#ifndef ROVER_HPP_
#define ROVER_HPP_

#include "../domain.hpp"
#include "../problem.hpp"
#include "../relations.hpp"


struct MyDomain {

	/*// typing
	Relation rover;
	Relation waypoint;
	Relation store;
	Relation at_soil_sample;
	
	// state
	Relation can_traverse;
	Relation available;
	Relation at;
	Relation visible;
	*/
	Action do_navigate;
	Action do_sample_soil;
	Action do_sample_rock;
	Action do_drop;
	Action do_calibrate;
	Action do_take_image;
	Action do_communicate_soil_data;
	Action do_communicate_rock_data;
	Action do_communicate_image_data;
	Action do_visit;
	Action do_unvisit;
	
	Method empty_store;
	Method navigate2;
	Method navigate3;
	Method send_soil_data;
	Method get_soil_data;
	Method send_rock_data;
	Method get_rock_data;
	Method send_image_data;
	Method get_image_data;
	Method calibrate;
	
	MyDomain();
};

// (defdomain basic (
MyDomain::MyDomain() :
	do_navigate("do_navigate"),
	do_sample_soil("do_sample_soil"),
	do_sample_rock("do_sample_rock"),
	do_drop("do_drop"),
	do_calibrate("do_calibrate"),
	do_take_image("do_take_image"),
	do_communicate_soil_data("do_communicate_soil_data"),
	do_communicate_rock_data("do_communicate_rock_data"),
	do_communicate_image_data("do_communicate_image_data"),
	do_visit("do_visit"),
	do_unvisit("do_unvisit"),
	
	empty_store("empty_store"),
	navigate2("navigate2"),
	navigate3("navigate3"),
	send_soil_data("send_soil_data"),
	get_soil_data("get_soil_data"),
	send_rock_data("send_rock_data"),
	get_rock_data("get_rock_data"),
	send_image_data("send_image_data"),
	get_image_data("get_image_data"),
	calibrate("calibrate")
{
	do_navigate.param("x");
	do_navigate.param("y");
	do_navigate.param("z");
	do_navigate.pre(
		rover("x") && waypoint("y") && waypoint("z") &&
		can_traversie("x", "y", "z") && available("x") && at("x", "y") &&
		visible("y", "z")
	);
	do_navigate.del(at("x", "y"));
	do_navigate.add(at("x", "z"));
	
	do_sample_soil.param("x");
	do_sample_soil.param("s");
	do_sample_soil.param("p");
	do_sample_soil.pre(
		rover("x") && store("s") && waypoint("p") && at("x", "p") &&
		at_soil_sample("p") && equipped_for_soil_analysis("x") &&
		store_of("s", "x") && empty("s")
	);
	do_sample_soil.del(empty("s"));
	do_sample_soil.del(at_soil_sample("p"));
	do_sample_soil.add(full("s"));
	do_sample_soil.add(have_soil_analysis("x", "p"));
	
	do_sample_rock.param("x");
	do_sample_rock.param("s");
	do_sample_rock.param("p");
	do_sample_rock.pre(
		rover("x") && store("s") && waypoint("p") && at("x", "p") &&
		at_rock_sample("p") && equipped_for_rock_analysis("x") &&
		store_of("s", "x") && empty("s")
	);
	do_sample_rock.del(empty("s"));
	do_sample_rock.del(at_rock_sample("p"));
	do_sample_rock.add(full("s"));
	do_sample_rock.add(have_rock_analysis("x", "p"));
	
	do_drop.param("x");
	do_drop.param("y");
	do_drop.pre(
		rover("x") && store("y") && store_of("y", "x") && full("y")
	);
	do_drop.del(full("y"));
	do_drop.add(empty("y"));
	
	do_calibrate.param("r");
	do_calibrate.param("i");
	do_calibrate.param("t");
	do_calibrate.param("w");
	do_calibrate.pre(
		rover("r") && camera("i") && objective("t") && waypoint("w") && 
		equipped_for_imaging("r") && calibration_target("i", "t") && 
		at("r", "w") && visible_from("t", "w") && on_board("i", "r")
	);
	do_calibrate.add(calibrated("i", "r"));
	
	do_take_image.param("r");
	do_take_image.param("p");
	do_take_image.param("o");
	do_take_image.param("i");
	do_take_image.param("m");
	do_take_image.pre(
		rover("r") && waypoint("p") && objective("o") && camera("i") && 
		mode("m") && calibrated("i", "r") && on_board("i", "r") &&
		equipped_for_imaging("r") && supports("i", "m") && 
		visible_from("o", "p") && at("r", "p")
	);
	do_take_image.del(calibrated("i", "r"));
	do_take_image.add(have_image("r", "o", "m"));
	
	do_communicate_soil_data.param("r");
	do_communicate_soil_data.param("l");
	do_communicate_soil_data.param("p");
	do_communicate_soil_data.param("x");
	do_communicate_soil_data.param("y");
	do_communicate_soil_data.pre(
		rover("r") && lander("l") && waypoint("p") && waypoint("x") &&
		waypoint("y") && at("r", "x") && at_lander("l", "y") &&
		have_soil_analysis("r", "p") && visible("x", "y") &&
		available("r") && channel_free("l")
	);
	do_communicate_soil_data.del(available("r"));
	do_communicate_soil_data.del(channel_free("l"));
	do_communicate_soil_data.add(channel_free("l"));
	do_communicate_soil_data.add(communicated_soil_data("p"));
	do_communicate_soil_data.add(available("r"));
	
	do_communicate_rock_data.param("r");
	do_communicate_rock_data.param("l");
	do_communicate_rock_data.param("p");
	do_communicate_rock_data.param("x");
	do_communicate_rock_data.param("y");
	do_communicate_rock_data.pre(
		rover("r") && lander("l") && waypoint("p") && waypoint("x") &&
		waypoint("y") && at("r", "x") && at_lander("l", "y") &&
	    have_rock_analysis("r", "p") && visible("x", "y") &&
	    available("r") && channel_free("l")
	);
	do_communicate_rock_data.del(available("r"));
	do_communicate_rock_data.del(channel_free("l"));
    do_communicate_rock_data.add(channel_free("l"));
	do_communicate_rock_data.add(communicated_rock_data("p"));
    do_communicate_rock_data.add(available("r"));
	
	do_communicate_image_data.param("r");
	do_communicate_image_data.param("l");
	do_communicate_image_data.param("o");
	do_communicate_image_data.param("m");
	do_communicate_image_data.param("x");
	do_communicate_image_data.param("y");
	do_communicate_image_data.pre(
		rover("r") && lander("l") && objective("o") && mode("m") &&
		waypoint("x") && waypoint("y") && at("r", "x") && 
		at_lander("l", "y") && have_image("r", "o", "m") &&
		visible("x", "y") && available("r") && channel_free("l")
	);
	do_communicate_image_data.del(available("r"));
	do_communicate_image_data.del(channel_free("l"));
    do_communicate_image_data.add(channel_free("l"));
	do_communicate_image_data.add(communicated_image_data("o", "m"));
    do_communicate_image_data.add(available("r"));
	
	empty_store.param("s");
	empty_store.param("rover");
	empty_store.alternative("Case1", empty("s"));
	empty_store.alternative("Case2", True, do_drop("rover", "s"));
	
	do_visit.param("waypoint");
	do_visit.add(visited("waypoint"));
	
	do_unvisit.param("waypoint");
	do_unvisit.del(visited("waypoint"));
	
	navigate2.param("rover");
	navigate2.param("to");
	navigate2.alternative(
		"navigate",
		at("rover", "from"),
		do_visit("from") >> navigate("rover", "from", "to") >> do_unvisite("from")
	);
	
	navigate3.param("rover");
	navigate3.param("from");
	navigate3.param("to");
	navigate3.alternative(
		"Case1",
		at("rover", "to")
	);
	navigate3.alternative(
		"Case2",
		can_traverse("rover", "from", "to"),
		do_navigate("rover", "from", "to")
	);
	navigate3.alternative(
		"Case3",
		waypoint("mid") &&
		can_traverse("rover", "from", "mid") &&
		!visited("mid"),
		do_navigate("rover", "from", "mid") >> visit("mid") >> do_navigate("rover", "mid", "to") >> unvisit("mid")
	);
		
	
	/*
	// (:operator (!pickup ?a) () () ((have ?a)))
	pickup.param("a");
	pickup.add(have("a"));

	// (:operator (!drop ?a) ((have ?a)) ((have ?a)) ())
	drop.param("a");
	drop.pre(have("a"));
	drop.del(have("a"));
	
	swap.param("x");
	swap.param("y");
	swap.alternative(have("x") && !have("y"), drop("x") >> pickup("y"));
	swap.alternative(have("y") && !have("x"), drop("y") >> pickup("x"));

	trocFor.param("x");
	trocFor.alternative(have("y") && !have("x") && provide("p", "x"), swap("x", "y"));*/
}

struct MyProblem: MyDomain, Problem {

	MyProblem() {
		/*add(have("kiwi"));
		add(have("lasagnes"));
		add(provide("toto", "banjo"));
		goal(trocFor("banjo"));*/
	}

};

#endif // ROVER_HPP_
