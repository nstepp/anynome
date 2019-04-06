function assigned = assign_responses(stimuli_times, response_times)
% function assigned = assign_responses(stimuli_times, response_times)
%
% Takes each stimulus event and assigns exactly one response event to it.
% Only the first of multiple responses is taken, and stimuli with no
% responses are dropped.
%
% Nigel Stepp <stepp@atistar.net>
% $Id: assign_responses.m 134 2007-08-30 02:05:23Z stepp $

% The algorithm:
% - place left boundary mid-way between current and previous event
% - same for right boundary with next event
% - the first response within this area is assigned to the current
%   event


assigned = [];

for i=2:length(stimuli_times)-1
	% get the "voronoi" interval for this event
	left_bound = mean([stimuli_times(i-1) stimuli_times(i)]);
	right_bound = mean([stimuli_times(i) stimuli_times(i+1)]);
	
	%count the number of responses in this interval
	in_bounds = find(response_times >= left_bound & response_times < right_bound, 1);
	
	if isempty(in_bounds) 
		disp(['Found interval with no responses: ' num2str(left_bound) ':' num2str(right_bound)]);
	else
		assigned(end+1,:) = [ stimuli_times(i) (response_times(in_bounds)-stimuli_times(i)) ];
	end;
end;

