
minimizable = Vue.component('minimizable', {
	template: '#minimizable-template',
	props: {
		default: {
			type: String,
			required: false,
			default: "min"
		}
	},
	data: function() {
		var defaultState = this.default == 'max' ? false : true;
		return {
			minimized: defaultState
		};
	}
});

var introPage = new Vue({
	el: '#introduction-page',
	components: {minimizable: minimizable},
});
