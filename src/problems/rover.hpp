#ifndef ROVER_HPP_
#define ROVER_HPP_

#include "../domain.hpp"
#include "../problem.hpp"
#include "../relations.hpp"


struct MyDomain {

	// typing
	Relation lander;
	Relation mode;
	Relation rover;
	Relation store;
	Relation waypoint;
	Relation camera;
	Relation objective;
	
	// state
	Relation visited;
	Relation visible;
	Relation at_soil_sample;
	Relation at_rock_sample;
	Relation at_lander;
	Relation channel_free;
	Relation at;
	Relation available;
	Relation store_of;
	Relation empty;
	Relation full;
	Relation equipped_for_rock_analysis;
	Relation have_rock_analysis;
	Relation communicated_rock_data;
	Relation equipped_for_soil_analysis;
	Relation have_soil_analysis;
	Relation communicated_soil_data;
	Relation equipped_for_imaging;
	Relation have_image;
	Relation communicated_image_data;
	Relation can_traverse;
	Relation on_board;
	Relation calibration_target;
	Relation calibrated;
	Relation supports;
	Relation visible_from;
	
	// operators
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
	
	// methods
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

MyDomain::MyDomain() :
	lander("lander", 1),
	mode("mode", 1),
	rover("rover", 1),
	store("store", 1),
	waypoint("waypoint", 1),
	camera("camera", 1),
	objective("objective", 1),
	
	visited("visited", 1),
	visible("visible", 2),
	at_soil_sample("at_soil_sample", 1),
	at_rock_sample("at_rock_sample", 1),
	at_lander("at_lander", 2),
	channel_free("channel_free", 1),
	at("at", 2),
	available("available", 1),
	store_of("store_of", 2),
	empty("empty", 1),
	full("full", 1),
	equipped_for_rock_analysis("equipped_for_rock_analysis", 1),
	have_rock_analysis("have_rock_analysis", 2),
	communicated_rock_data("communicated_rock_data", 1),
	equipped_for_soil_analysis("equipped_for_soil_analysis", 1),
	have_soil_analysis("have_soil_analysis", 2),
	communicated_soil_data("communicated_soil_data", 1),
	equipped_for_imaging("equipped_for_imaging", 1),
	have_image("have_image", 3),
	communicated_image_data("communicated_image_data", 2),
	can_traverse("can_traverse", 3),
	on_board("on_board", 2),
	calibration_target("calibration_target", 2),
	calibrated("calibrated", 2),
	supports("supports", 2),
	visible_from("visible_from", 2),
	
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
		can_traverse("x", "y", "z") && available("x") && at("x", "y") &&
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
		do_navigate("rover", "from", "mid") >> do_visit("mid") >> navigate3("rover", "mid", "to") >> do_unvisit("mid")
	);
	
	navigate2.param("rover");
	navigate2.param("to");
	navigate2.alternative(
		"navigate",
		at("rover", "from"),
		do_visit("from") >> navigate3("rover", "from", "to") >> do_unvisit("from")
	);
	
	send_soil_data.param("rover");
	send_soil_data.param("waypoint");
	send_soil_data.alternative(
		"send_soil_data",
		lander("l") && at_lander("l", "y") && visible("x", "y"),
		navigate2("rover", "x") >> do_communicate_soil_data("rover", "l", "waypoint", "x", "y")
	);
	
	get_soil_data.param("waypoint");
	get_soil_data.alternative(
		"get_soil_data",
		waypoint("waypoint") && rover("rover") && store_of("s", "rover") && equipped_for_soil_analysis("rover"),
		navigate2("rover", "waypoint") >> empty_store("s", "rover") >> do_sample_soil("rover", "s", "waypoint") >> send_soil_data("rover", "waypoint")
	);
	
	send_rock_data.param("rover");
	send_rock_data.param("waypoint");
	send_rock_data.alternative(
		"send_rock_data",
		lander("l") && at_lander("l", "y") && visible("x", "y"),
		navigate2("rover", "x") >> do_communicate_rock_data("rover", "l", "waypoint", "x", "y")
	);
	
	get_rock_data.param("waypoint");
	get_rock_data.alternative(
		"get_rock_data",
		waypoint("waypoint") && rover("rover") && equipped_for_rock_analysis("rover") && store_of("s", "rover"),
		navigate2("rover", "waypoint") >> empty_store("s", "rover") >> do_sample_rock("rover", "s", "waypoint") >> send_rock_data("rover", "waypoint")
	);
	
	send_image_data.param("rover");
	send_image_data.param("objective");
	send_image_data.param("mode");
	send_image_data.alternative(
		"send_image_data",
		lander("l") && at_lander("l", "y") && visible("x", "y"),
		navigate2("rover", "x") >> do_communicate_image_data("rover", "l", "objective", "mode", "x", "y")
	);
	
	calibrate.param("rover");
	calibrate.param("camera");
	calibrate.alternative(
		"calibrate",
		calibration_target("camera", "objective") && visible_from("objective", "waypoint"),
		navigate2("rover", "waypoint") >> do_calibrate("rover", "camera", "objective", "waypoint")
	);
	
	get_image_data.param("objective");
	get_image_data.param("mode");
	get_image_data.alternative(
		"get_image_data",
		objective("objective") && equipped_for_imaging("rover") && on_board("camera", "rover") && supports("camera", "mode") && visible_from("objective", "waypoint"),
		calibrate("rover", "camera") >> navigate2("rover", "waypoint") >> do_take_image("rover", "waypoint", "objective", "camera", "mode") >> send_image_data("rover", "objective", "mode")
	);
}

struct MyProblem: MyDomain, Problem {

	MyProblem() {
		// facts
		add(lander("general"));
		add(mode("colour"));
		add(mode("high_res"));
		add(mode("low_res"));
		add(rover("rover0"));
		add(rover("rover1"));
		add(rover("rover2"));
		add(rover("rover3"));
		add(rover("rover4"));
		add(rover("rover5"));
		add(rover("rover6"));
		add(rover("rover7"));
		add(store("rover0store"));
		add(store("rover1store"));
		add(store("rover2store"));
		add(store("rover3store"));
		add(store("rover4store"));
		add(store("rover5store"));
		add(store("rover6store"));
		add(store("rover7store"));
		add(waypoint("waypoint0"));
		add(waypoint("waypoint1"));
		add(waypoint("waypoint2"));
		add(waypoint("waypoint3"));
		add(waypoint("waypoint4"));
		add(waypoint("waypoint5"));
		add(waypoint("waypoint6"));
		add(waypoint("waypoint7"));
		add(waypoint("waypoint8"));
		add(waypoint("waypoint9"));
		add(waypoint("waypoint10"));
		add(waypoint("waypoint11"));
		add(waypoint("waypoint12"));
		add(waypoint("waypoint13"));
		add(waypoint("waypoint14"));
		add(waypoint("waypoint15"));
		add(waypoint("waypoint16"));
		add(waypoint("waypoint17"));
		add(waypoint("waypoint18"));
		add(waypoint("waypoint19"));
		add(waypoint("waypoint20"));
		add(waypoint("waypoint21"));
		add(waypoint("waypoint22"));
		add(waypoint("waypoint23"));
		add(waypoint("waypoint24"));
		add(waypoint("waypoint25"));
		add(waypoint("waypoint26"));
		add(waypoint("waypoint27"));
		add(waypoint("waypoint28"));
		add(waypoint("waypoint29"));
		add(camera("camera0"));
		add(camera("camera1"));
		add(camera("camera2"));
		add(camera("camera3"));
		add(camera("camera4"));
		add(camera("camera5"));
		add(camera("camera6"));
		add(camera("camera7"));
		add(camera("camera8"));
		add(camera("camera9"));
		add(camera("camera10"));
		add(objective("objective0"));
		add(objective("objective1"));
		add(objective("objective2"));
		add(objective("objective3"));
		add(objective("objective4"));
		add(objective("objective5"));
		add(objective("objective6"));
		add(objective("objective7"));
		add(objective("objective8"));
		add(objective("objective9"));
		// initial states
		// TODO: rewrite visible using equivalent relation
		add(visible("waypoint0", "waypoint18"));
		add(visible("waypoint18", "waypoint0"));
		add(visible("waypoint0", "waypoint21"));
		add(visible("waypoint21", "waypoint0"));
		add(visible("waypoint0", "waypoint22"));
		add(visible("waypoint22", "waypoint0"));
		add(visible("waypoint1", "waypoint0"));
		add(visible("waypoint0", "waypoint1"));
		add(visible("waypoint1", "waypoint6"));
		add(visible("waypoint6", "waypoint1"));
		add(visible("waypoint1", "waypoint16"));
		add(visible("waypoint16", "waypoint1"));
		add(visible("waypoint1", "waypoint26"));
		add(visible("waypoint26", "waypoint1"));
		add(visible("waypoint1", "waypoint27"));
		add(visible("waypoint27", "waypoint1"));
		add(visible("waypoint2", "waypoint6"));
		add(visible("waypoint6", "waypoint2"));
		add(visible("waypoint2", "waypoint8"));
		add(visible("waypoint8", "waypoint2"));
		add(visible("waypoint2", "waypoint11"));
		add(visible("waypoint11", "waypoint2"));
		add(visible("waypoint2", "waypoint21"));
		add(visible("waypoint21", "waypoint2"));
		add(visible("waypoint2", "waypoint24"));
		add(visible("waypoint24", "waypoint2"));
		add(visible("waypoint2", "waypoint27"));
		add(visible("waypoint27", "waypoint2"));
		add(visible("waypoint3", "waypoint7"));
		add(visible("waypoint7", "waypoint3"));
		add(visible("waypoint3", "waypoint13"));
		add(visible("waypoint13", "waypoint3"));
		add(visible("waypoint3", "waypoint26"));
		add(visible("waypoint26", "waypoint3"));
		add(visible("waypoint4", "waypoint2"));
		add(visible("waypoint2", "waypoint4"));
		add(visible("waypoint4", "waypoint6"));
		add(visible("waypoint6", "waypoint4"));
		add(visible("waypoint4", "waypoint8"));
		add(visible("waypoint8", "waypoint4"));
		add(visible("waypoint4", "waypoint9"));
		add(visible("waypoint9", "waypoint4"));
		add(visible("waypoint4", "waypoint13"));
		add(visible("waypoint13", "waypoint4"));
		add(visible("waypoint4", "waypoint14"));
		add(visible("waypoint14", "waypoint4"));
		add(visible("waypoint5", "waypoint1"));
		add(visible("waypoint1", "waypoint5"));
		add(visible("waypoint5", "waypoint2"));
		add(visible("waypoint2", "waypoint5"));
		add(visible("waypoint5", "waypoint8"));
		add(visible("waypoint8", "waypoint5"));
		add(visible("waypoint5", "waypoint27"));
		add(visible("waypoint27", "waypoint5"));
		add(visible("waypoint6", "waypoint11"));
		add(visible("waypoint11", "waypoint6"));
		add(visible("waypoint6", "waypoint15"));
		add(visible("waypoint15", "waypoint6"));
		add(visible("waypoint6", "waypoint18"));
		add(visible("waypoint18", "waypoint6"));
		add(visible("waypoint6", "waypoint25"));
		add(visible("waypoint25", "waypoint6"));
		add(visible("waypoint6", "waypoint27"));
		add(visible("waypoint27", "waypoint6"));
		add(visible("waypoint7", "waypoint5"));
		add(visible("waypoint5", "waypoint7"));
		add(visible("waypoint7", "waypoint23"));
		add(visible("waypoint23", "waypoint7"));
		add(visible("waypoint8", "waypoint0"));
		add(visible("waypoint0", "waypoint8"));
		add(visible("waypoint8", "waypoint13"));
		add(visible("waypoint13", "waypoint8"));
		add(visible("waypoint8", "waypoint18"));
		add(visible("waypoint18", "waypoint8"));
		add(visible("waypoint8", "waypoint22"));
		add(visible("waypoint22", "waypoint8"));
		add(visible("waypoint8", "waypoint26"));
		add(visible("waypoint26", "waypoint8"));
		add(visible("waypoint8", "waypoint28"));
		add(visible("waypoint28", "waypoint8"));
		add(visible("waypoint9", "waypoint1"));
		add(visible("waypoint1", "waypoint9"));
		add(visible("waypoint9", "waypoint10"));
		add(visible("waypoint10", "waypoint9"));
		add(visible("waypoint9", "waypoint20"));
		add(visible("waypoint20", "waypoint9"));
		add(visible("waypoint9", "waypoint25"));
		add(visible("waypoint25", "waypoint9"));
		add(visible("waypoint10", "waypoint1"));
		add(visible("waypoint1", "waypoint10"));
		add(visible("waypoint10", "waypoint7"));
		add(visible("waypoint7", "waypoint10"));
		add(visible("waypoint10", "waypoint12"));
		add(visible("waypoint12", "waypoint10"));
		add(visible("waypoint10", "waypoint13"));
		add(visible("waypoint13", "waypoint10"));
		add(visible("waypoint10", "waypoint18"));
		add(visible("waypoint18", "waypoint10"));
		add(visible("waypoint10", "waypoint23"));
		add(visible("waypoint23", "waypoint10"));
		add(visible("waypoint10", "waypoint27"));
		add(visible("waypoint27", "waypoint10"));
		add(visible("waypoint10", "waypoint28"));
		add(visible("waypoint28", "waypoint10"));
		add(visible("waypoint11", "waypoint8"));
		add(visible("waypoint8", "waypoint11"));
		add(visible("waypoint11", "waypoint10"));
		add(visible("waypoint10", "waypoint11"));
		add(visible("waypoint11", "waypoint18"));
		add(visible("waypoint18", "waypoint11"));
		add(visible("waypoint12", "waypoint3"));
		add(visible("waypoint3", "waypoint12"));
		add(visible("waypoint12", "waypoint7"));
		add(visible("waypoint7", "waypoint12"));
		add(visible("waypoint12", "waypoint23"));
		add(visible("waypoint23", "waypoint12"));
		add(visible("waypoint13", "waypoint7"));
		add(visible("waypoint7", "waypoint13"));
		add(visible("waypoint13", "waypoint15"));
		add(visible("waypoint15", "waypoint13"));
		add(visible("waypoint13", "waypoint21"));
		add(visible("waypoint21", "waypoint13"));
		add(visible("waypoint13", "waypoint23"));
		add(visible("waypoint23", "waypoint13"));
		add(visible("waypoint14", "waypoint1"));
		add(visible("waypoint1", "waypoint14"));
		add(visible("waypoint14", "waypoint6"));
		add(visible("waypoint6", "waypoint14"));
		add(visible("waypoint14", "waypoint20"));
		add(visible("waypoint20", "waypoint14"));
		add(visible("waypoint15", "waypoint0"));
		add(visible("waypoint0", "waypoint15"));
		add(visible("waypoint15", "waypoint25"));
		add(visible("waypoint25", "waypoint15"));
		add(visible("waypoint15", "waypoint26"));
		add(visible("waypoint26", "waypoint15"));
		add(visible("waypoint15", "waypoint28"));
		add(visible("waypoint28", "waypoint15"));
		add(visible("waypoint16", "waypoint10"));
		add(visible("waypoint10", "waypoint16"));
		add(visible("waypoint16", "waypoint11"));
		add(visible("waypoint11", "waypoint16"));
		add(visible("waypoint16", "waypoint12"));
		add(visible("waypoint12", "waypoint16"));
		add(visible("waypoint16", "waypoint14"));
		add(visible("waypoint14", "waypoint16"));
		add(visible("waypoint16", "waypoint17"));
		add(visible("waypoint17", "waypoint16"));
		add(visible("waypoint16", "waypoint18"));
		add(visible("waypoint18", "waypoint16"));
		add(visible("waypoint16", "waypoint26"));
		add(visible("waypoint26", "waypoint16"));
		add(visible("waypoint17", "waypoint1"));
		add(visible("waypoint1", "waypoint17"));
		add(visible("waypoint17", "waypoint4"));
		add(visible("waypoint4", "waypoint17"));
		add(visible("waypoint17", "waypoint18"));
		add(visible("waypoint18", "waypoint17"));
		add(visible("waypoint17", "waypoint20"));
		add(visible("waypoint20", "waypoint17"));
		add(visible("waypoint18", "waypoint2"));
		add(visible("waypoint2", "waypoint18"));
		add(visible("waypoint18", "waypoint23"));
		add(visible("waypoint23", "waypoint18"));
		add(visible("waypoint18", "waypoint24"));
		add(visible("waypoint24", "waypoint18"));
		add(visible("waypoint19", "waypoint0"));
		add(visible("waypoint0", "waypoint19"));
		add(visible("waypoint19", "waypoint2"));
		add(visible("waypoint2", "waypoint19"));
		add(visible("waypoint19", "waypoint9"));
		add(visible("waypoint9", "waypoint19"));
		add(visible("waypoint19", "waypoint12"));
		add(visible("waypoint12", "waypoint19"));
		add(visible("waypoint19", "waypoint18"));
		add(visible("waypoint18", "waypoint19"));
		add(visible("waypoint19", "waypoint24"));
		add(visible("waypoint24", "waypoint19"));
		add(visible("waypoint20", "waypoint0"));
		add(visible("waypoint0", "waypoint20"));
		add(visible("waypoint20", "waypoint7"));
		add(visible("waypoint7", "waypoint20"));
		add(visible("waypoint20", "waypoint23"));
		add(visible("waypoint23", "waypoint20"));
		add(visible("waypoint20", "waypoint26"));
		add(visible("waypoint26", "waypoint20"));
		add(visible("waypoint21", "waypoint4"));
		add(visible("waypoint4", "waypoint21"));
		add(visible("waypoint21", "waypoint5"));
		add(visible("waypoint5", "waypoint21"));
		add(visible("waypoint21", "waypoint12"));
		add(visible("waypoint12", "waypoint21"));
		add(visible("waypoint21", "waypoint14"));
		add(visible("waypoint14", "waypoint21"));
		add(visible("waypoint21", "waypoint15"));
		add(visible("waypoint15", "waypoint21"));
		add(visible("waypoint21", "waypoint18"));
		add(visible("waypoint18", "waypoint21"));
		add(visible("waypoint21", "waypoint19"));
		add(visible("waypoint19", "waypoint21"));
		add(visible("waypoint21", "waypoint24"));
		add(visible("waypoint24", "waypoint21"));
		add(visible("waypoint21", "waypoint26"));
		add(visible("waypoint26", "waypoint21"));
		add(visible("waypoint22", "waypoint2"));
		add(visible("waypoint2", "waypoint22"));
		add(visible("waypoint22", "waypoint26"));
		add(visible("waypoint26", "waypoint22"));
		add(visible("waypoint23", "waypoint16"));
		add(visible("waypoint16", "waypoint23"));
		add(visible("waypoint23", "waypoint17"));
		add(visible("waypoint17", "waypoint23"));
		add(visible("waypoint24", "waypoint0"));
		add(visible("waypoint0", "waypoint24"));
		add(visible("waypoint24", "waypoint1"));
		add(visible("waypoint1", "waypoint24"));
		add(visible("waypoint24", "waypoint5"));
		add(visible("waypoint5", "waypoint24"));
		add(visible("waypoint24", "waypoint12"));
		add(visible("waypoint12", "waypoint24"));
		add(visible("waypoint24", "waypoint16"));
		add(visible("waypoint16", "waypoint24"));
		add(visible("waypoint25", "waypoint0"));
		add(visible("waypoint0", "waypoint25"));
		add(visible("waypoint25", "waypoint19"));
		add(visible("waypoint19", "waypoint25"));
		add(visible("waypoint25", "waypoint20"));
		add(visible("waypoint20", "waypoint25"));
		add(visible("waypoint26", "waypoint2"));
		add(visible("waypoint2", "waypoint26"));
		add(visible("waypoint26", "waypoint14"));
		add(visible("waypoint14", "waypoint26"));
		add(visible("waypoint26", "waypoint28"));
		add(visible("waypoint28", "waypoint26"));
		add(visible("waypoint26", "waypoint29"));
		add(visible("waypoint29", "waypoint26"));
		add(visible("waypoint27", "waypoint9"));
		add(visible("waypoint9", "waypoint27"));
		add(visible("waypoint27", "waypoint25"));
		add(visible("waypoint25", "waypoint27"));
		add(visible("waypoint27", "waypoint28"));
		add(visible("waypoint28", "waypoint27"));
		add(visible("waypoint28", "waypoint3"));
		add(visible("waypoint3", "waypoint28"));
		add(visible("waypoint28", "waypoint5"));
		add(visible("waypoint5", "waypoint28"));
		add(visible("waypoint28", "waypoint9"));
		add(visible("waypoint9", "waypoint28"));
		add(visible("waypoint28", "waypoint18"));
		add(visible("waypoint18", "waypoint28"));
		add(visible("waypoint29", "waypoint0"));
		add(visible("waypoint0", "waypoint29"));
		add(visible("waypoint29", "waypoint20"));
		add(visible("waypoint20", "waypoint29"));
		add(at_soil_sample("waypoint0"));
		add(at_rock_sample("waypoint0"));
		add(at_rock_sample("waypoint1"));
		add(at_soil_sample("waypoint2"));
		add(at_rock_sample("waypoint3"));
		add(at_soil_sample("waypoint4"));
		add(at_rock_sample("waypoint4"));
		add(at_soil_sample("waypoint5"));
		add(at_rock_sample("waypoint5"));
		add(at_rock_sample("waypoint6"));
		add(at_soil_sample("waypoint7"));
		add(at_rock_sample("waypoint7"));
		add(at_rock_sample("waypoint10"));
		add(at_soil_sample("waypoint12"));
		add(at_soil_sample("waypoint13"));
		add(at_soil_sample("waypoint14"));
		add(at_soil_sample("waypoint15"));
		add(at_rock_sample("waypoint15"));
		add(at_rock_sample("waypoint18"));
		add(at_rock_sample("waypoint19"));
		add(at_soil_sample("waypoint20"));
		add(at_rock_sample("waypoint20"));
		add(at_rock_sample("waypoint21"));
		add(at_rock_sample("waypoint22"));
		add(at_soil_sample("waypoint23"));
		add(at_soil_sample("waypoint26"));
		add(at_rock_sample("waypoint27"));
		add(at_lander("general", "waypoint17"));
		add(channel_free("general"));
		add(at("rover0", "waypoint2"));
		add(available("rover0"));
		add(store_of("rover0store", "rover0"));
		add(empty("rover0store"));
		add(equipped_for_rock_analysis("rover0"));
		add(can_traverse("rover0", "waypoint2", "waypoint4"));
		add(can_traverse("rover0", "waypoint4", "waypoint2"));
		add(can_traverse("rover0", "waypoint2", "waypoint5"));
		add(can_traverse("rover0", "waypoint5", "waypoint2"));
		add(can_traverse("rover0", "waypoint2", "waypoint6"));
		add(can_traverse("rover0", "waypoint6", "waypoint2"));
		add(can_traverse("rover0", "waypoint2", "waypoint11"));
		add(can_traverse("rover0", "waypoint11", "waypoint2"));
		add(can_traverse("rover0", "waypoint2", "waypoint19"));
		add(can_traverse("rover0", "waypoint19", "waypoint2"));
		add(can_traverse("rover0", "waypoint2", "waypoint22"));
		add(can_traverse("rover0", "waypoint22", "waypoint2"));
		add(can_traverse("rover0", "waypoint2", "waypoint24"));
		add(can_traverse("rover0", "waypoint24", "waypoint2"));
		add(can_traverse("rover0", "waypoint2", "waypoint27"));
		add(can_traverse("rover0", "waypoint27", "waypoint2"));
		add(can_traverse("rover0", "waypoint4", "waypoint8"));
		add(can_traverse("rover0", "waypoint8", "waypoint4"));
		add(can_traverse("rover0", "waypoint4", "waypoint9"));
		add(can_traverse("rover0", "waypoint9", "waypoint4"));
		add(can_traverse("rover0", "waypoint4", "waypoint13"));
		add(can_traverse("rover0", "waypoint13", "waypoint4"));
		add(can_traverse("rover0", "waypoint4", "waypoint17"));
		add(can_traverse("rover0", "waypoint17", "waypoint4"));
		add(can_traverse("rover0", "waypoint4", "waypoint21"));
		add(can_traverse("rover0", "waypoint21", "waypoint4"));
		add(can_traverse("rover0", "waypoint5", "waypoint7"));
		add(can_traverse("rover0", "waypoint7", "waypoint5"));
		add(can_traverse("rover0", "waypoint5", "waypoint28"));
		add(can_traverse("rover0", "waypoint28", "waypoint5"));
		add(can_traverse("rover0", "waypoint6", "waypoint14"));
		add(can_traverse("rover0", "waypoint14", "waypoint6"));
		add(can_traverse("rover0", "waypoint6", "waypoint25"));
		add(can_traverse("rover0", "waypoint25", "waypoint6"));
		add(can_traverse("rover0", "waypoint11", "waypoint10"));
		add(can_traverse("rover0", "waypoint10", "waypoint11"));
		add(can_traverse("rover0", "waypoint11", "waypoint16"));
		add(can_traverse("rover0", "waypoint16", "waypoint11"));
		add(can_traverse("rover0", "waypoint19", "waypoint0"));
		add(can_traverse("rover0", "waypoint0", "waypoint19"));
		add(can_traverse("rover0", "waypoint19", "waypoint12"));
		add(can_traverse("rover0", "waypoint12", "waypoint19"));
		add(can_traverse("rover0", "waypoint19", "waypoint18"));
		add(can_traverse("rover0", "waypoint18", "waypoint19"));
		add(can_traverse("rover0", "waypoint22", "waypoint26"));
		add(can_traverse("rover0", "waypoint26", "waypoint22"));
		add(can_traverse("rover0", "waypoint24", "waypoint1"));
		add(can_traverse("rover0", "waypoint1", "waypoint24"));
		add(can_traverse("rover0", "waypoint9", "waypoint20"));
		add(can_traverse("rover0", "waypoint20", "waypoint9"));
		add(can_traverse("rover0", "waypoint13", "waypoint15"));
		add(can_traverse("rover0", "waypoint15", "waypoint13"));
		add(can_traverse("rover0", "waypoint7", "waypoint3"));
		add(can_traverse("rover0", "waypoint3", "waypoint7"));
		add(can_traverse("rover0", "waypoint7", "waypoint23"));
		add(can_traverse("rover0", "waypoint23", "waypoint7"));
		add(at("rover1", "waypoint5"));
		add(available("rover1"));
		add(store_of("rover1store", "rover1"));
		add(empty("rover1store"));
		add(equipped_for_rock_analysis("rover1"));
		add(equipped_for_imaging("rover1"));
		add(can_traverse("rover1", "waypoint5", "waypoint1"));
		add(can_traverse("rover1", "waypoint1", "waypoint5"));
		add(can_traverse("rover1", "waypoint5", "waypoint2"));
		add(can_traverse("rover1", "waypoint2", "waypoint5"));
		add(can_traverse("rover1", "waypoint5", "waypoint8"));
		add(can_traverse("rover1", "waypoint8", "waypoint5"));
		add(can_traverse("rover1", "waypoint5", "waypoint21"));
		add(can_traverse("rover1", "waypoint21", "waypoint5"));
		add(can_traverse("rover1", "waypoint5", "waypoint24"));
		add(can_traverse("rover1", "waypoint24", "waypoint5"));
		add(can_traverse("rover1", "waypoint5", "waypoint27"));
		add(can_traverse("rover1", "waypoint27", "waypoint5"));
		add(can_traverse("rover1", "waypoint1", "waypoint0"));
		add(can_traverse("rover1", "waypoint0", "waypoint1"));
		add(can_traverse("rover1", "waypoint1", "waypoint6"));
		add(can_traverse("rover1", "waypoint6", "waypoint1"));
		add(can_traverse("rover1", "waypoint1", "waypoint9"));
		add(can_traverse("rover1", "waypoint9", "waypoint1"));
		add(can_traverse("rover1", "waypoint1", "waypoint10"));
		add(can_traverse("rover1", "waypoint10", "waypoint1"));
		add(can_traverse("rover1", "waypoint1", "waypoint16"));
		add(can_traverse("rover1", "waypoint16", "waypoint1"));
		add(can_traverse("rover1", "waypoint1", "waypoint17"));
		add(can_traverse("rover1", "waypoint17", "waypoint1"));
		add(can_traverse("rover1", "waypoint1", "waypoint26"));
		add(can_traverse("rover1", "waypoint26", "waypoint1"));
		add(can_traverse("rover1", "waypoint2", "waypoint4"));
		add(can_traverse("rover1", "waypoint4", "waypoint2"));
		add(can_traverse("rover1", "waypoint2", "waypoint11"));
		add(can_traverse("rover1", "waypoint11", "waypoint2"));
		add(can_traverse("rover1", "waypoint2", "waypoint18"));
		add(can_traverse("rover1", "waypoint18", "waypoint2"));
		add(can_traverse("rover1", "waypoint2", "waypoint19"));
		add(can_traverse("rover1", "waypoint19", "waypoint2"));
		add(can_traverse("rover1", "waypoint2", "waypoint22"));
		add(can_traverse("rover1", "waypoint22", "waypoint2"));
		add(can_traverse("rover1", "waypoint8", "waypoint28"));
		add(can_traverse("rover1", "waypoint28", "waypoint8"));
		add(can_traverse("rover1", "waypoint21", "waypoint12"));
		add(can_traverse("rover1", "waypoint12", "waypoint21"));
		add(can_traverse("rover1", "waypoint21", "waypoint13"));
		add(can_traverse("rover1", "waypoint13", "waypoint21"));
		add(can_traverse("rover1", "waypoint21", "waypoint14"));
		add(can_traverse("rover1", "waypoint14", "waypoint21"));
		add(can_traverse("rover1", "waypoint21", "waypoint15"));
		add(can_traverse("rover1", "waypoint15", "waypoint21"));
		add(can_traverse("rover1", "waypoint27", "waypoint25"));
		add(can_traverse("rover1", "waypoint25", "waypoint27"));
		add(can_traverse("rover1", "waypoint0", "waypoint20"));
		add(can_traverse("rover1", "waypoint20", "waypoint0"));
		add(can_traverse("rover1", "waypoint0", "waypoint29"));
		add(can_traverse("rover1", "waypoint29", "waypoint0"));
		add(can_traverse("rover1", "waypoint10", "waypoint7"));
		add(can_traverse("rover1", "waypoint7", "waypoint10"));
		add(can_traverse("rover1", "waypoint10", "waypoint23"));
		add(can_traverse("rover1", "waypoint23", "waypoint10"));
		add(can_traverse("rover1", "waypoint26", "waypoint3"));
		add(can_traverse("rover1", "waypoint3", "waypoint26"));
		add(at("rover2", "waypoint18"));
		add(available("rover2"));
		add(store_of("rover2store", "rover2"));
		add(empty("rover2store"));
		add(equipped_for_rock_analysis("rover2"));
		add(equipped_for_imaging("rover2"));
		add(can_traverse("rover2", "waypoint18", "waypoint0"));
		add(can_traverse("rover2", "waypoint0", "waypoint18"));
		add(can_traverse("rover2", "waypoint18", "waypoint6"));
		add(can_traverse("rover2", "waypoint6", "waypoint18"));
		add(can_traverse("rover2", "waypoint18", "waypoint10"));
		add(can_traverse("rover2", "waypoint10", "waypoint18"));
		add(can_traverse("rover2", "waypoint18", "waypoint16"));
		add(can_traverse("rover2", "waypoint16", "waypoint18"));
		add(can_traverse("rover2", "waypoint18", "waypoint17"));
		add(can_traverse("rover2", "waypoint17", "waypoint18"));
		add(can_traverse("rover2", "waypoint18", "waypoint21"));
		add(can_traverse("rover2", "waypoint21", "waypoint18"));
		add(can_traverse("rover2", "waypoint18", "waypoint23"));
		add(can_traverse("rover2", "waypoint23", "waypoint18"));
		add(can_traverse("rover2", "waypoint18", "waypoint24"));
		add(can_traverse("rover2", "waypoint24", "waypoint18"));
		add(can_traverse("rover2", "waypoint18", "waypoint28"));
		add(can_traverse("rover2", "waypoint28", "waypoint18"));
		add(can_traverse("rover2", "waypoint0", "waypoint1"));
		add(can_traverse("rover2", "waypoint1", "waypoint0"));
		add(can_traverse("rover2", "waypoint0", "waypoint8"));
		add(can_traverse("rover2", "waypoint8", "waypoint0"));
		add(can_traverse("rover2", "waypoint0", "waypoint15"));
		add(can_traverse("rover2", "waypoint15", "waypoint0"));
		add(can_traverse("rover2", "waypoint0", "waypoint19"));
		add(can_traverse("rover2", "waypoint19", "waypoint0"));
		add(can_traverse("rover2", "waypoint0", "waypoint22"));
		add(can_traverse("rover2", "waypoint22", "waypoint0"));
		add(can_traverse("rover2", "waypoint0", "waypoint25"));
		add(can_traverse("rover2", "waypoint25", "waypoint0"));
		add(can_traverse("rover2", "waypoint6", "waypoint2"));
		add(can_traverse("rover2", "waypoint2", "waypoint6"));
		add(can_traverse("rover2", "waypoint6", "waypoint4"));
		add(can_traverse("rover2", "waypoint4", "waypoint6"));
		add(can_traverse("rover2", "waypoint6", "waypoint11"));
		add(can_traverse("rover2", "waypoint11", "waypoint6"));
		add(can_traverse("rover2", "waypoint6", "waypoint27"));
		add(can_traverse("rover2", "waypoint27", "waypoint6"));
		add(can_traverse("rover2", "waypoint10", "waypoint9"));
		add(can_traverse("rover2", "waypoint9", "waypoint10"));
		add(can_traverse("rover2", "waypoint10", "waypoint13"));
		add(can_traverse("rover2", "waypoint13", "waypoint10"));
		add(can_traverse("rover2", "waypoint16", "waypoint12"));
		add(can_traverse("rover2", "waypoint12", "waypoint16"));
		add(can_traverse("rover2", "waypoint16", "waypoint14"));
		add(can_traverse("rover2", "waypoint14", "waypoint16"));
		add(can_traverse("rover2", "waypoint16", "waypoint26"));
		add(can_traverse("rover2", "waypoint26", "waypoint16"));
		add(can_traverse("rover2", "waypoint17", "waypoint20"));
		add(can_traverse("rover2", "waypoint20", "waypoint17"));
		add(can_traverse("rover2", "waypoint21", "waypoint5"));
		add(can_traverse("rover2", "waypoint5", "waypoint21"));
		add(can_traverse("rover2", "waypoint23", "waypoint7"));
		add(can_traverse("rover2", "waypoint7", "waypoint23"));
		add(can_traverse("rover2", "waypoint28", "waypoint3"));
		add(can_traverse("rover2", "waypoint3", "waypoint28"));
		add(at("rover3", "waypoint27"));
		add(available("rover3"));
		add(store_of("rover3store", "rover3"));
		add(empty("rover3store"));
		add(equipped_for_rock_analysis("rover3"));
		add(equipped_for_imaging("rover3"));
		add(can_traverse("rover3", "waypoint27", "waypoint1"));
		add(can_traverse("rover3", "waypoint1", "waypoint27"));
		add(can_traverse("rover3", "waypoint27", "waypoint6"));
		add(can_traverse("rover3", "waypoint6", "waypoint27"));
		add(can_traverse("rover3", "waypoint27", "waypoint9"));
		add(can_traverse("rover3", "waypoint9", "waypoint27"));
		add(can_traverse("rover3", "waypoint27", "waypoint10"));
		add(can_traverse("rover3", "waypoint10", "waypoint27"));
		add(can_traverse("rover3", "waypoint27", "waypoint25"));
		add(can_traverse("rover3", "waypoint25", "waypoint27"));
		add(can_traverse("rover3", "waypoint1", "waypoint0"));
		add(can_traverse("rover3", "waypoint0", "waypoint1"));
		add(can_traverse("rover3", "waypoint1", "waypoint5"));
		add(can_traverse("rover3", "waypoint5", "waypoint1"));
		add(can_traverse("rover3", "waypoint1", "waypoint16"));
		add(can_traverse("rover3", "waypoint16", "waypoint1"));
		add(can_traverse("rover3", "waypoint1", "waypoint17"));
		add(can_traverse("rover3", "waypoint17", "waypoint1"));
		add(can_traverse("rover3", "waypoint1", "waypoint24"));
		add(can_traverse("rover3", "waypoint24", "waypoint1"));
		add(can_traverse("rover3", "waypoint1", "waypoint26"));
		add(can_traverse("rover3", "waypoint26", "waypoint1"));
		add(can_traverse("rover3", "waypoint6", "waypoint2"));
		add(can_traverse("rover3", "waypoint2", "waypoint6"));
		add(can_traverse("rover3", "waypoint6", "waypoint14"));
		add(can_traverse("rover3", "waypoint14", "waypoint6"));
		add(can_traverse("rover3", "waypoint6", "waypoint15"));
		add(can_traverse("rover3", "waypoint15", "waypoint6"));
		add(can_traverse("rover3", "waypoint9", "waypoint4"));
		add(can_traverse("rover3", "waypoint4", "waypoint9"));
		add(can_traverse("rover3", "waypoint9", "waypoint19"));
		add(can_traverse("rover3", "waypoint19", "waypoint9"));
		add(can_traverse("rover3", "waypoint9", "waypoint20"));
		add(can_traverse("rover3", "waypoint20", "waypoint9"));
		add(can_traverse("rover3", "waypoint9", "waypoint28"));
		add(can_traverse("rover3", "waypoint28", "waypoint9"));
		add(can_traverse("rover3", "waypoint10", "waypoint7"));
		add(can_traverse("rover3", "waypoint7", "waypoint10"));
		add(can_traverse("rover3", "waypoint10", "waypoint11"));
		add(can_traverse("rover3", "waypoint11", "waypoint10"));
		add(can_traverse("rover3", "waypoint10", "waypoint18"));
		add(can_traverse("rover3", "waypoint18", "waypoint10"));
		add(can_traverse("rover3", "waypoint10", "waypoint23"));
		add(can_traverse("rover3", "waypoint23", "waypoint10"));
		add(can_traverse("rover3", "waypoint0", "waypoint8"));
		add(can_traverse("rover3", "waypoint8", "waypoint0"));
		add(can_traverse("rover3", "waypoint0", "waypoint21"));
		add(can_traverse("rover3", "waypoint21", "waypoint0"));
		add(can_traverse("rover3", "waypoint0", "waypoint22"));
		add(can_traverse("rover3", "waypoint22", "waypoint0"));
		add(can_traverse("rover3", "waypoint0", "waypoint29"));
		add(can_traverse("rover3", "waypoint29", "waypoint0"));
		add(can_traverse("rover3", "waypoint16", "waypoint12"));
		add(can_traverse("rover3", "waypoint12", "waypoint16"));
		add(can_traverse("rover3", "waypoint26", "waypoint3"));
		add(can_traverse("rover3", "waypoint3", "waypoint26"));
		add(can_traverse("rover3", "waypoint4", "waypoint13"));
		add(can_traverse("rover3", "waypoint13", "waypoint4"));
		add(at("rover4", "waypoint7"));
		add(available("rover4"));
		add(store_of("rover4store", "rover4"));
		add(empty("rover4store"));
		add(equipped_for_rock_analysis("rover4"));
		add(equipped_for_imaging("rover4"));
		add(can_traverse("rover4", "waypoint7", "waypoint3"));
		add(can_traverse("rover4", "waypoint3", "waypoint7"));
		add(can_traverse("rover4", "waypoint7", "waypoint5"));
		add(can_traverse("rover4", "waypoint5", "waypoint7"));
		add(can_traverse("rover4", "waypoint7", "waypoint10"));
		add(can_traverse("rover4", "waypoint10", "waypoint7"));
		add(can_traverse("rover4", "waypoint7", "waypoint12"));
		add(can_traverse("rover4", "waypoint12", "waypoint7"));
		add(can_traverse("rover4", "waypoint7", "waypoint13"));
		add(can_traverse("rover4", "waypoint13", "waypoint7"));
		add(can_traverse("rover4", "waypoint7", "waypoint23"));
		add(can_traverse("rover4", "waypoint23", "waypoint7"));
		add(can_traverse("rover4", "waypoint3", "waypoint26"));
		add(can_traverse("rover4", "waypoint26", "waypoint3"));
		add(can_traverse("rover4", "waypoint5", "waypoint2"));
		add(can_traverse("rover4", "waypoint2", "waypoint5"));
		add(can_traverse("rover4", "waypoint5", "waypoint21"));
		add(can_traverse("rover4", "waypoint21", "waypoint5"));
		add(can_traverse("rover4", "waypoint5", "waypoint27"));
		add(can_traverse("rover4", "waypoint27", "waypoint5"));
		add(can_traverse("rover4", "waypoint5", "waypoint28"));
		add(can_traverse("rover4", "waypoint28", "waypoint5"));
		add(can_traverse("rover4", "waypoint10", "waypoint18"));
		add(can_traverse("rover4", "waypoint18", "waypoint10"));
		add(can_traverse("rover4", "waypoint12", "waypoint16"));
		add(can_traverse("rover4", "waypoint16", "waypoint12"));
		add(can_traverse("rover4", "waypoint13", "waypoint4"));
		add(can_traverse("rover4", "waypoint4", "waypoint13"));
		add(can_traverse("rover4", "waypoint13", "waypoint15"));
		add(can_traverse("rover4", "waypoint15", "waypoint13"));
		add(can_traverse("rover4", "waypoint23", "waypoint17"));
		add(can_traverse("rover4", "waypoint17", "waypoint23"));
		add(can_traverse("rover4", "waypoint23", "waypoint20"));
		add(can_traverse("rover4", "waypoint20", "waypoint23"));
		add(can_traverse("rover4", "waypoint26", "waypoint1"));
		add(can_traverse("rover4", "waypoint1", "waypoint26"));
		add(can_traverse("rover4", "waypoint26", "waypoint14"));
		add(can_traverse("rover4", "waypoint14", "waypoint26"));
		add(can_traverse("rover4", "waypoint26", "waypoint22"));
		add(can_traverse("rover4", "waypoint22", "waypoint26"));
		add(can_traverse("rover4", "waypoint2", "waypoint6"));
		add(can_traverse("rover4", "waypoint6", "waypoint2"));
		add(can_traverse("rover4", "waypoint2", "waypoint11"));
		add(can_traverse("rover4", "waypoint11", "waypoint2"));
		add(can_traverse("rover4", "waypoint2", "waypoint19"));
		add(can_traverse("rover4", "waypoint19", "waypoint2"));
		add(can_traverse("rover4", "waypoint2", "waypoint24"));
		add(can_traverse("rover4", "waypoint24", "waypoint2"));
		add(can_traverse("rover4", "waypoint21", "waypoint0"));
		add(can_traverse("rover4", "waypoint0", "waypoint21"));
		add(can_traverse("rover4", "waypoint27", "waypoint25"));
		add(can_traverse("rover4", "waypoint25", "waypoint27"));
		add(can_traverse("rover4", "waypoint28", "waypoint8"));
		add(can_traverse("rover4", "waypoint8", "waypoint28"));
		add(can_traverse("rover4", "waypoint28", "waypoint9"));
		add(can_traverse("rover4", "waypoint9", "waypoint28"));
		add(can_traverse("rover4", "waypoint20", "waypoint29"));
		add(can_traverse("rover4", "waypoint29", "waypoint20"));
		add(at("rover5", "waypoint7"));
		add(available("rover5"));
		add(store_of("rover5store", "rover5"));
		add(empty("rover5store"));
		add(equipped_for_soil_analysis("rover5"));
		add(equipped_for_imaging("rover5"));
		add(can_traverse("rover5", "waypoint7", "waypoint3"));
		add(can_traverse("rover5", "waypoint3", "waypoint7"));
		add(can_traverse("rover5", "waypoint7", "waypoint10"));
		add(can_traverse("rover5", "waypoint10", "waypoint7"));
		add(can_traverse("rover5", "waypoint7", "waypoint12"));
		add(can_traverse("rover5", "waypoint12", "waypoint7"));
		add(can_traverse("rover5", "waypoint7", "waypoint23"));
		add(can_traverse("rover5", "waypoint23", "waypoint7"));
		add(can_traverse("rover5", "waypoint3", "waypoint13"));
		add(can_traverse("rover5", "waypoint13", "waypoint3"));
		add(can_traverse("rover5", "waypoint3", "waypoint26"));
		add(can_traverse("rover5", "waypoint26", "waypoint3"));
		add(can_traverse("rover5", "waypoint3", "waypoint28"));
		add(can_traverse("rover5", "waypoint28", "waypoint3"));
		add(can_traverse("rover5", "waypoint10", "waypoint9"));
		add(can_traverse("rover5", "waypoint9", "waypoint10"));
		add(can_traverse("rover5", "waypoint10", "waypoint11"));
		add(can_traverse("rover5", "waypoint11", "waypoint10"));
		add(can_traverse("rover5", "waypoint10", "waypoint16"));
		add(can_traverse("rover5", "waypoint16", "waypoint10"));
		add(can_traverse("rover5", "waypoint10", "waypoint18"));
		add(can_traverse("rover5", "waypoint18", "waypoint10"));
		add(can_traverse("rover5", "waypoint10", "waypoint27"));
		add(can_traverse("rover5", "waypoint27", "waypoint10"));
		add(can_traverse("rover5", "waypoint12", "waypoint19"));
		add(can_traverse("rover5", "waypoint19", "waypoint12"));
		add(can_traverse("rover5", "waypoint12", "waypoint21"));
		add(can_traverse("rover5", "waypoint21", "waypoint12"));
		add(can_traverse("rover5", "waypoint23", "waypoint17"));
		add(can_traverse("rover5", "waypoint17", "waypoint23"));
		add(can_traverse("rover5", "waypoint23", "waypoint20"));
		add(can_traverse("rover5", "waypoint20", "waypoint23"));
		add(can_traverse("rover5", "waypoint13", "waypoint4"));
		add(can_traverse("rover5", "waypoint4", "waypoint13"));
		add(can_traverse("rover5", "waypoint13", "waypoint8"));
		add(can_traverse("rover5", "waypoint8", "waypoint13"));
		add(can_traverse("rover5", "waypoint26", "waypoint1"));
		add(can_traverse("rover5", "waypoint1", "waypoint26"));
		add(can_traverse("rover5", "waypoint26", "waypoint2"));
		add(can_traverse("rover5", "waypoint2", "waypoint26"));
		add(can_traverse("rover5", "waypoint26", "waypoint14"));
		add(can_traverse("rover5", "waypoint14", "waypoint26"));
		add(can_traverse("rover5", "waypoint26", "waypoint15"));
		add(can_traverse("rover5", "waypoint15", "waypoint26"));
		add(can_traverse("rover5", "waypoint26", "waypoint22"));
		add(can_traverse("rover5", "waypoint22", "waypoint26"));
		add(can_traverse("rover5", "waypoint26", "waypoint29"));
		add(can_traverse("rover5", "waypoint29", "waypoint26"));
		add(can_traverse("rover5", "waypoint11", "waypoint6"));
		add(can_traverse("rover5", "waypoint6", "waypoint11"));
		add(can_traverse("rover5", "waypoint16", "waypoint24"));
		add(can_traverse("rover5", "waypoint24", "waypoint16"));
		add(can_traverse("rover5", "waypoint18", "waypoint0"));
		add(can_traverse("rover5", "waypoint0", "waypoint18"));
		add(can_traverse("rover5", "waypoint19", "waypoint25"));
		add(can_traverse("rover5", "waypoint25", "waypoint19"));
		add(can_traverse("rover5", "waypoint21", "waypoint5"));
		add(can_traverse("rover5", "waypoint5", "waypoint21"));
		add(at("rover6", "waypoint23"));
		add(available("rover6"));
		add(store_of("rover6store", "rover6"));
		add(empty("rover6store"));
		add(equipped_for_soil_analysis("rover6"));
		add(equipped_for_rock_analysis("rover6"));
		add(can_traverse("rover6", "waypoint23", "waypoint7"));
		add(can_traverse("rover6", "waypoint7", "waypoint23"));
		add(can_traverse("rover6", "waypoint23", "waypoint12"));
		add(can_traverse("rover6", "waypoint12", "waypoint23"));
		add(can_traverse("rover6", "waypoint23", "waypoint13"));
		add(can_traverse("rover6", "waypoint13", "waypoint23"));
		add(can_traverse("rover6", "waypoint23", "waypoint17"));
		add(can_traverse("rover6", "waypoint17", "waypoint23"));
		add(can_traverse("rover6", "waypoint23", "waypoint18"));
		add(can_traverse("rover6", "waypoint18", "waypoint23"));
		add(can_traverse("rover6", "waypoint7", "waypoint5"));
		add(can_traverse("rover6", "waypoint5", "waypoint7"));
		add(can_traverse("rover6", "waypoint7", "waypoint10"));
		add(can_traverse("rover6", "waypoint10", "waypoint7"));
		add(can_traverse("rover6", "waypoint7", "waypoint20"));
		add(can_traverse("rover6", "waypoint20", "waypoint7"));
		add(can_traverse("rover6", "waypoint12", "waypoint3"));
		add(can_traverse("rover6", "waypoint3", "waypoint12"));
		add(can_traverse("rover6", "waypoint12", "waypoint24"));
		add(can_traverse("rover6", "waypoint24", "waypoint12"));
		add(can_traverse("rover6", "waypoint13", "waypoint4"));
		add(can_traverse("rover6", "waypoint4", "waypoint13"));
		add(can_traverse("rover6", "waypoint13", "waypoint21"));
		add(can_traverse("rover6", "waypoint21", "waypoint13"));
		add(can_traverse("rover6", "waypoint17", "waypoint16"));
		add(can_traverse("rover6", "waypoint16", "waypoint17"));
		add(can_traverse("rover6", "waypoint18", "waypoint0"));
		add(can_traverse("rover6", "waypoint0", "waypoint18"));
		add(can_traverse("rover6", "waypoint18", "waypoint6"));
		add(can_traverse("rover6", "waypoint6", "waypoint18"));
		add(can_traverse("rover6", "waypoint18", "waypoint19"));
		add(can_traverse("rover6", "waypoint19", "waypoint18"));
		add(can_traverse("rover6", "waypoint18", "waypoint28"));
		add(can_traverse("rover6", "waypoint28", "waypoint18"));
		add(can_traverse("rover6", "waypoint5", "waypoint2"));
		add(can_traverse("rover6", "waypoint2", "waypoint5"));
		add(can_traverse("rover6", "waypoint5", "waypoint27"));
		add(can_traverse("rover6", "waypoint27", "waypoint5"));
		add(can_traverse("rover6", "waypoint10", "waypoint9"));
		add(can_traverse("rover6", "waypoint9", "waypoint10"));
		add(can_traverse("rover6", "waypoint10", "waypoint11"));
		add(can_traverse("rover6", "waypoint11", "waypoint10"));
		add(can_traverse("rover6", "waypoint20", "waypoint25"));
		add(can_traverse("rover6", "waypoint25", "waypoint20"));
		add(can_traverse("rover6", "waypoint20", "waypoint26"));
		add(can_traverse("rover6", "waypoint26", "waypoint20"));
		add(can_traverse("rover6", "waypoint20", "waypoint29"));
		add(can_traverse("rover6", "waypoint29", "waypoint20"));
		add(can_traverse("rover6", "waypoint4", "waypoint8"));
		add(can_traverse("rover6", "waypoint8", "waypoint4"));
		add(at("rover7", "waypoint12"));
		add(available("rover7"));
		add(store_of("rover7store", "rover7"));
		add(empty("rover7store"));
		add(equipped_for_soil_analysis("rover7"));
		add(equipped_for_imaging("rover7"));
		add(can_traverse("rover7", "waypoint12", "waypoint3"));
		add(can_traverse("rover7", "waypoint3", "waypoint12"));
		add(can_traverse("rover7", "waypoint12", "waypoint7"));
		add(can_traverse("rover7", "waypoint7", "waypoint12"));
		add(can_traverse("rover7", "waypoint12", "waypoint10"));
		add(can_traverse("rover7", "waypoint10", "waypoint12"));
		add(can_traverse("rover7", "waypoint12", "waypoint16"));
		add(can_traverse("rover7", "waypoint16", "waypoint12"));
		add(can_traverse("rover7", "waypoint12", "waypoint19"));
		add(can_traverse("rover7", "waypoint19", "waypoint12"));
		add(can_traverse("rover7", "waypoint12", "waypoint23"));
		add(can_traverse("rover7", "waypoint23", "waypoint12"));
		add(can_traverse("rover7", "waypoint12", "waypoint24"));
		add(can_traverse("rover7", "waypoint24", "waypoint12"));
		add(can_traverse("rover7", "waypoint3", "waypoint13"));
		add(can_traverse("rover7", "waypoint13", "waypoint3"));
		add(can_traverse("rover7", "waypoint3", "waypoint26"));
		add(can_traverse("rover7", "waypoint26", "waypoint3"));
		add(can_traverse("rover7", "waypoint3", "waypoint28"));
		add(can_traverse("rover7", "waypoint28", "waypoint3"));
		add(can_traverse("rover7", "waypoint7", "waypoint5"));
		add(can_traverse("rover7", "waypoint5", "waypoint7"));
		add(can_traverse("rover7", "waypoint7", "waypoint20"));
		add(can_traverse("rover7", "waypoint20", "waypoint7"));
		add(can_traverse("rover7", "waypoint10", "waypoint9"));
		add(can_traverse("rover7", "waypoint9", "waypoint10"));
		add(can_traverse("rover7", "waypoint10", "waypoint11"));
		add(can_traverse("rover7", "waypoint11", "waypoint10"));
		add(can_traverse("rover7", "waypoint10", "waypoint18"));
		add(can_traverse("rover7", "waypoint18", "waypoint10"));
		add(can_traverse("rover7", "waypoint10", "waypoint27"));
		add(can_traverse("rover7", "waypoint27", "waypoint10"));
		add(can_traverse("rover7", "waypoint16", "waypoint1"));
		add(can_traverse("rover7", "waypoint1", "waypoint16"));
		add(can_traverse("rover7", "waypoint19", "waypoint0"));
		add(can_traverse("rover7", "waypoint0", "waypoint19"));
		add(can_traverse("rover7", "waypoint19", "waypoint2"));
		add(can_traverse("rover7", "waypoint2", "waypoint19"));
		add(can_traverse("rover7", "waypoint19", "waypoint21"));
		add(can_traverse("rover7", "waypoint21", "waypoint19"));
		add(can_traverse("rover7", "waypoint23", "waypoint17"));
		add(can_traverse("rover7", "waypoint17", "waypoint23"));
		add(can_traverse("rover7", "waypoint13", "waypoint8"));
		add(can_traverse("rover7", "waypoint8", "waypoint13"));
		add(can_traverse("rover7", "waypoint13", "waypoint15"));
		add(can_traverse("rover7", "waypoint15", "waypoint13"));
		add(can_traverse("rover7", "waypoint26", "waypoint14"));
		add(can_traverse("rover7", "waypoint14", "waypoint26"));
		add(can_traverse("rover7", "waypoint26", "waypoint22"));
		add(can_traverse("rover7", "waypoint22", "waypoint26"));
		add(can_traverse("rover7", "waypoint26", "waypoint29"));
		add(can_traverse("rover7", "waypoint29", "waypoint26"));
		add(can_traverse("rover7", "waypoint20", "waypoint25"));
		add(can_traverse("rover7", "waypoint25", "waypoint20"));
		add(can_traverse("rover7", "waypoint11", "waypoint6"));
		add(can_traverse("rover7", "waypoint6", "waypoint11"));
		add(on_board("camera0", "rover3"));
		add(calibration_target("camera0", "objective5"));
		add(supports("camera0", "colour"));
		add(supports("camera0", "high_res"));
		add(on_board("camera1", "rover1"));
		add(calibration_target("camera1", "objective5"));
		add(supports("camera1", "colour"));
		add(supports("camera1", "high_res"));
		add(on_board("camera2", "rover5"));
		add(calibration_target("camera2", "objective4"));
		add(supports("camera2", "low_res"));
		add(on_board("camera3", "rover4"));
		add(calibration_target("camera3", "objective5"));
		add(supports("camera3", "low_res"));
		add(on_board("camera4", "rover3"));
		add(calibration_target("camera4", "objective3"));
		add(supports("camera4", "high_res"));
		add(on_board("camera5", "rover3"));
		add(calibration_target("camera5", "objective6"));
		add(supports("camera5", "colour"));
		add(supports("camera5", "high_res"));
		add(on_board("camera6", "rover2"));
		add(calibration_target("camera6", "objective4"));
		add(supports("camera6", "high_res"));
		add(on_board("camera7", "rover3"));
		add(calibration_target("camera7", "objective1"));
		add(supports("camera7", "high_res"));
		add(supports("camera7", "low_res"));
		add(on_board("camera8", "rover5"));
		add(calibration_target("camera8", "objective0"));
		add(supports("camera8", "colour"));
		add(on_board("camera9", "rover5"));
		add(calibration_target("camera9", "objective0"));
		add(supports("camera9", "high_res"));
		add(supports("camera9", "low_res"));
		add(on_board("camera10", "rover7"));
		add(calibration_target("camera10", "objective9"));
		add(supports("camera10", "high_res"));
		add(supports("camera10", "low_res"));
		add(visible_from("objective0", "waypoint0"));
		add(visible_from("objective0", "waypoint1"));
		add(visible_from("objective0", "waypoint2"));
		add(visible_from("objective0", "waypoint3"));
		add(visible_from("objective0", "waypoint4"));
		add(visible_from("objective0", "waypoint5"));
		add(visible_from("objective0", "waypoint6"));
		add(visible_from("objective0", "waypoint7"));
		add(visible_from("objective0", "waypoint8"));
		add(visible_from("objective0", "waypoint9"));
		add(visible_from("objective0", "waypoint10"));
		add(visible_from("objective0", "waypoint11"));
		add(visible_from("objective0", "waypoint12"));
		add(visible_from("objective0", "waypoint13"));
		add(visible_from("objective0", "waypoint14"));
		add(visible_from("objective0", "waypoint15"));
		add(visible_from("objective0", "waypoint16"));
		add(visible_from("objective0", "waypoint17"));
		add(visible_from("objective0", "waypoint18"));
		add(visible_from("objective1", "waypoint0"));
		add(visible_from("objective1", "waypoint1"));
		add(visible_from("objective1", "waypoint2"));
		add(visible_from("objective1", "waypoint3"));
		add(visible_from("objective1", "waypoint4"));
		add(visible_from("objective1", "waypoint5"));
		add(visible_from("objective1", "waypoint6"));
		add(visible_from("objective1", "waypoint7"));
		add(visible_from("objective1", "waypoint8"));
		add(visible_from("objective1", "waypoint9"));
		add(visible_from("objective1", "waypoint10"));
		add(visible_from("objective2", "waypoint0"));
		add(visible_from("objective2", "waypoint1"));
		add(visible_from("objective2", "waypoint2"));
		add(visible_from("objective2", "waypoint3"));
		add(visible_from("objective2", "waypoint4"));
		add(visible_from("objective2", "waypoint5"));
		add(visible_from("objective2", "waypoint6"));
		add(visible_from("objective2", "waypoint7"));
		add(visible_from("objective2", "waypoint8"));
		add(visible_from("objective2", "waypoint9"));
		add(visible_from("objective2", "waypoint10"));
		add(visible_from("objective2", "waypoint11"));
		add(visible_from("objective2", "waypoint12"));
		add(visible_from("objective2", "waypoint13"));
		add(visible_from("objective2", "waypoint14"));
		add(visible_from("objective2", "waypoint15"));
		add(visible_from("objective2", "waypoint16"));
		add(visible_from("objective2", "waypoint17"));
		add(visible_from("objective2", "waypoint18"));
		add(visible_from("objective2", "waypoint19"));
		add(visible_from("objective2", "waypoint20"));
		add(visible_from("objective2", "waypoint21"));
		add(visible_from("objective2", "waypoint22"));
		add(visible_from("objective2", "waypoint23"));
		add(visible_from("objective2", "waypoint24"));
		add(visible_from("objective2", "waypoint25"));
		add(visible_from("objective2", "waypoint26"));
		add(visible_from("objective2", "waypoint27"));
		add(visible_from("objective2", "waypoint28"));
		add(visible_from("objective2", "waypoint29"));
		add(visible_from("objective3", "waypoint0"));
		add(visible_from("objective3", "waypoint1"));
		add(visible_from("objective3", "waypoint2"));
		add(visible_from("objective3", "waypoint3"));
		add(visible_from("objective3", "waypoint4"));
		add(visible_from("objective3", "waypoint5"));
		add(visible_from("objective3", "waypoint6"));
		add(visible_from("objective3", "waypoint7"));
		add(visible_from("objective3", "waypoint8"));
		add(visible_from("objective3", "waypoint9"));
		add(visible_from("objective3", "waypoint10"));
		add(visible_from("objective3", "waypoint11"));
		add(visible_from("objective3", "waypoint12"));
		add(visible_from("objective3", "waypoint13"));
		add(visible_from("objective3", "waypoint14"));
		add(visible_from("objective3", "waypoint15"));
		add(visible_from("objective3", "waypoint16"));
		add(visible_from("objective3", "waypoint17"));
		add(visible_from("objective4", "waypoint0"));
		add(visible_from("objective4", "waypoint1"));
		add(visible_from("objective4", "waypoint2"));
		add(visible_from("objective4", "waypoint3"));
		add(visible_from("objective4", "waypoint4"));
		add(visible_from("objective4", "waypoint5"));
		add(visible_from("objective5", "waypoint0"));
		add(visible_from("objective5", "waypoint1"));
		add(visible_from("objective5", "waypoint2"));
		add(visible_from("objective5", "waypoint3"));
		add(visible_from("objective5", "waypoint4"));
		add(visible_from("objective5", "waypoint5"));
		add(visible_from("objective5", "waypoint6"));
		add(visible_from("objective5", "waypoint7"));
		add(visible_from("objective5", "waypoint8"));
		add(visible_from("objective5", "waypoint9"));
		add(visible_from("objective5", "waypoint10"));
		add(visible_from("objective6", "waypoint0"));
		add(visible_from("objective6", "waypoint1"));
		add(visible_from("objective6", "waypoint2"));
		add(visible_from("objective6", "waypoint3"));
		add(visible_from("objective6", "waypoint4"));
		add(visible_from("objective6", "waypoint5"));
		add(visible_from("objective6", "waypoint6"));
		add(visible_from("objective6", "waypoint7"));
		add(visible_from("objective6", "waypoint8"));
		add(visible_from("objective6", "waypoint9"));
		add(visible_from("objective6", "waypoint10"));
		add(visible_from("objective6", "waypoint11"));
		add(visible_from("objective6", "waypoint12"));
		add(visible_from("objective6", "waypoint13"));
		add(visible_from("objective6", "waypoint14"));
		add(visible_from("objective6", "waypoint15"));
		add(visible_from("objective7", "waypoint0"));
		add(visible_from("objective7", "waypoint1"));
		add(visible_from("objective7", "waypoint2"));
		add(visible_from("objective7", "waypoint3"));
		add(visible_from("objective8", "waypoint0"));
		add(visible_from("objective8", "waypoint1"));
		add(visible_from("objective8", "waypoint2"));
		add(visible_from("objective9", "waypoint0"));
		add(visible_from("objective9", "waypoint1"));
		add(visible_from("objective9", "waypoint2"));
		add(visible_from("objective9", "waypoint3"));
		add(visible_from("objective9", "waypoint4"));
		add(visible_from("objective9", "waypoint5"));
		add(visible_from("objective9", "waypoint6"));
		add(visible_from("objective9", "waypoint7"));
		add(visible_from("objective9", "waypoint8"));
		add(visible_from("objective9", "waypoint9"));
		
		// goals
		//goal(get_soil_data("waypoint5"));
		//goal(do_communicate_soil_data("rover5", "general", "waypoint5", "waypoint16", "waypoint17"));
		goal(
			/*get_soil_data("waypoint5")
			>> get_soil_data("waypoint13")
			>> get_soil_data("waypoint4")
			>>*/ get_soil_data("waypoint23")
			>> get_soil_data("waypoint15")
			/*>> get_soil_data("waypoint7")
			>> get_soil_data("waypoint20")
			>> get_soil_data("waypoint26")
			>> get_soil_data("waypoint2")
			>> get_rock_data("waypoint27")
			>> get_rock_data("waypoint19")
			>> get_rock_data("waypoint1")
			>> get_rock_data("waypoint3")
			>> get_rock_data("waypoint7")
			>> get_rock_data("waypoint20")
			>> get_rock_data("waypoint22")
			>> get_image_data("objective2", "low_res")
			>> get_image_data("objective6", "low_res")
			>> get_image_data("objective3", "low_res")
			>> get_image_data("objective6", "colour")
			>> get_image_data("objective3", "high_res")
			>> get_image_data("objective1", "colour")
			>> get_image_data("objective6", "high_res")
			>> get_image_data("objective3", "colour")
			>> get_image_data("objective2", "high_res")*/
		);
		
		/*
		JSHOP2 solution is for single goal get_soil_data("waypoint5") is:
		(!visit waypoint7)
		(!navigate rover5 waypoint7 waypoint12)
		(!visit waypoint12)
		(!navigate rover5 waypoint12 waypoint21)
		(!visit waypoint21)
		(!navigate rover5 waypoint21 waypoint5)
		(!unvisit waypoint21)
		(!unvisit waypoint12)
		(!unvisit waypoint7)
		(!sample_soil rover5 rover5store waypoint5)
		(!visit waypoint5)
		(!navigate rover5 waypoint5 waypoint21)
		(!visit waypoint21)
		(!navigate rover5 waypoint21 waypoint12)
		(!visit waypoint12)
		(!navigate rover5 waypoint12 waypoint7)
		(!visit waypoint7)
		(!navigate rover5 waypoint7 waypoint10)
		(!visit waypoint10)
		(!navigate rover5 waypoint10 waypoint16)
		(!unvisit waypoint10)
		(!unvisit waypoint7)
		(!unvisit waypoint12)
		(!unvisit waypoint21)
		(!unvisit waypoint5)
		(!communicate_soil_data rover5 general waypoint5 waypoint16 waypoint17)
		*/

	}
};

#endif // ROVER_HPP_
